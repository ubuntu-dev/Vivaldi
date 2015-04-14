#include "vivaldi.h"

#include <libxml/tree.h>

#include <string.h>

// Compile (on OS X--- not yet tested on Linux) with
//
//   clang -undefined dynamic_lookup -shared -fPIC -I../include
//         -I/usr/include/libxml2 -lxml2 -oxml.dylib xml.c

// Structure:
//
// class XmlDocument
//   fn init(string)
//   fn root() // XmlNode
//   fn to_str() // String
// end
//
// class XmlNode
//   fn init(): except
//   fn all(sym_or_string) // XmlNodeList
//   fn first(sym_or_string) // XmlNode | except
//   fn attr(sym_or_string) // String | nil
//   fn name() // String
//   fn contents() // String
// end
//
// class XmlNodeList
//   fn init(xml_node, sym_or_string)
//   fn size() // Int
//   fn at(int) // XmlNode
//   fn start() // XmlNodeListIterator
//   fn stop() // XmlNodeListIterator
// end
//
// class XmlNodeListIterator
//  fn init(): except
//  fn get() // XmlNode
//  fn equals(xml_node_list_iterator) // Bool
//  fn unequal(xml_node_list_iterator) // Bool
//  fn increment() // XmlNodeListIterator
// end

static const char* xml_get_str(vv_object_t str_or_sym);

static vv_object_t xml_type_document;
static vv_object_t xml_type_node;
static vv_object_t xml_type_node_list;
static vv_object_t xml_type_node_list_iterator;

static vv_object_t xml_document_ctor(void);
static void xml_document_dtor(void*);
static vv_object_t xml_document_init(vv_object_t self, vv_object_t text);
static vv_object_t xml_document_root(vv_object_t self);
static vv_object_t xml_document_to_str(vv_object_t self);

static vv_object_t xml_node_from(xmlNode* nd, vv_object_t doc);

static vv_object_t xml_node_ctor(void);
static vv_object_t xml_node_all(vv_object_t self, vv_object_t name);
static vv_object_t xml_node_first(vv_object_t self, vv_object_t name);
static vv_object_t xml_node_attr(vv_object_t self, vv_object_t name);
static vv_object_t xml_node_name(vv_object_t self);
static vv_object_t xml_node_contents(vv_object_t self);
static vv_object_t xml_node_str(vv_object_t self);

typedef struct xml_c_node_list {
  xmlNode** nodes;
  size_t size;
  size_t capacity;
} xml_c_node_list;

static void xml_node_list_append(xml_c_node_list* list, xmlNode* node);
static xml_c_node_list* xml_node_list_new();
static void xml_node_list_free(xml_c_node_list* list);

static vv_object_t xml_node_list_ctor(void);
static void xml_node_list_dtor(void* blob);
static vv_object_t xml_node_list_init(vv_object_t self);
static vv_object_t xml_node_list_size(vv_object_t self);
static vv_object_t xml_node_list_at(vv_object_t self, vv_object_t index);
static vv_object_t xml_node_list_start(vv_object_t self);
static vv_object_t xml_node_list_stop(vv_object_t self);

typedef struct xml_c_node_list_iter {
  xml_c_node_list* list;
  size_t position;
} xml_c_node_list_iter;

static vv_object_t xml_node_list_iter_from(xml_c_node_list* list,
                                            size_t idx,
                                            vv_object_t node_list);
static vv_object_t xml_node_list_iter_ctor(void);
static void xml_node_list_iter_dtor(void* blob);
static vv_object_t xml_node_list_iter_get(vv_object_t self);
static vv_object_t xml_node_list_iter_equals(vv_object_t self,
                                              vv_object_t other);
static vv_object_t xml_node_list_iter_unequal(vv_object_t self,
                                               vv_object_t other);
static vv_object_t xml_node_list_iter_increment(vv_object_t self);

static void xml_cleanup(void);
void vv_init_lib(void);

static const char*
xml_get_str(vv_object_t str_or_sym)
{
  const char* str;
  if (vv_get_type(str_or_sym) == vv_builtin_type_symbol) {
    vv_symbol_t sym;
    int success = vv_get_symbol(str_or_sym, &sym);
    if (success == -1) {
      return NULL;
    }
    str = sym.string;
  }
  else {
    int success = vv_get_str(str_or_sym, &str);
    if (success == -1) {
      return NULL;
    }
  }
  return str;
}

static vv_object_t
xml_document_ctor(void)
{
  return vv_alloc_blob(NULL, xml_document_dtor);
}

static void
xml_document_dtor(void* blob)
{
  xmlDoc* doc = blob;
  xmlFreeDoc(doc);
}

static vv_object_t
xml_document_init(vv_object_t self, vv_object_t text)
{
  const char* string;
  int str_succ = vv_get_str(text, &string);
  if (str_succ == -1) {
    return vv_null;
  }
  size_t len = strlen(string);

  xmlDoc* doc = xmlReadMemory(string, len, "noname.xml", NULL, 0);
  int success = vv_set_blob(self, doc);
  if (doc == NULL || success == -1) {
    return vv_null;
  }

  return self;
}

static vv_object_t
xml_document_to_str(vv_object_t self)
{
  xmlDoc* doc;
  int success = vv_get_blob(self, (void**)&doc);
  if (success == -1) {
    return vv_null;
  }
  xmlChar* string;
  int string_sz;
  xmlDocDumpFormatMemory(doc, &string, &string_sz, 1);

  vv_object_t vv_str = vv_new_string((char*)string);
  xmlFree(string);
  return vv_str;
}

static vv_object_t
xml_document_root(vv_object_t self)
{
  xmlDoc* doc;
  int success = vv_get_blob(self, (void**)&doc);
  if (success == -1) {
    return vv_null;
  }
  return xml_node_from(xmlDocGetRootElement(doc), self);
}

static vv_object_t
xml_node_ctor(void)
{
  return vv_null;
}

static vv_object_t
xml_node_from(xmlNode* node, vv_object_t doc)
{
  vv_object_t obj = vv_new_blob(node, NULL, xml_type_node);
  if (!obj) {
    return vv_null;
  }
  vv_symbol_t sym = vv_make_symbol("_doc");
  vv_object_t success = vv_set_mem(obj, sym, doc);
  if (success == vv_null) {
    return vv_null;
  }
  return obj;
}

static vv_object_t
xml_node_name(vv_object_t self)
{
  xmlNode* node;
  int success = vv_get_blob(self, (void**)&node);
  if (success == -1) {
    return vv_null;
  }

  return vv_new_string((const char*)node->name);
}

static vv_object_t
xml_node_contents(vv_object_t self)
{
  xmlNode* node;
  int success = vv_get_blob(self, (void**)&node);
  if (success == -1) {
    return vv_null;
  }

  xmlDoc* doc;
  vv_object_t doc_obj = vv_get_mem(self, vv_make_symbol("_doc"));
  if (!doc_obj) {
    return vv_null;
  }
  int doc_success = vv_get_blob(doc_obj, (void**)&doc);
  if (doc_success == -1) {
    return vv_null;
  }

  xmlChar* string = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
  vv_object_t str = vv_new_string((const char*)string);
  xmlFree(string);
  return str;
}
static vv_object_t xml_node_str(vv_object_t self);
static vv_object_t
xml_node_all(vv_object_t self, vv_object_t name)
{
  vv_object_t args[2] = { self, name };
  vv_object_t obj = vv_new_object(xml_type_node_list, args, 2);
  return obj;
}

static vv_object_t
xml_node_first(vv_object_t self, vv_object_t name)
{
  const char* str = xml_get_str(name);
  if (!str) {
    return vv_null;
  }

  xmlNode* node;
  int success = vv_get_blob(self, (void**)&node);
  if (success == -1) {
    return vv_null;
  }

  const xmlChar* xml_str = (const xmlChar*) str;

  for (xmlNode* i = node->children; i; i = i->next) {
    if (xmlStrcmp(i->name, xml_str) == 0) {
      vv_object_t doc = vv_get_mem(self, vv_make_symbol("_doc"));
      if (!doc) {
        return vv_null;
      }
      return xml_node_from(i, doc);
    }
  }
  return vv_null;
}

static vv_object_t
xml_node_attr(vv_object_t self, vv_object_t name)
{
  const char* str = xml_get_str(name);
  if (!str) {
    return vv_null;
  }

  xmlNode* node;
  int success = vv_get_blob(self, (void**)&node);
  if (success == -1) {
    return vv_null;
  }

  xmlChar* attr = xmlGetProp(node, (const xmlChar*)str);
  if (!attr) {
    return vv_null;
  }

  vv_object_t attr_obj = vv_new_string((const char*)attr);
  xmlFree(attr);

  return attr_obj;
}

static vv_object_t
xml_node_str(vv_object_t self)
{
  xmlNode* node;
  int success = vv_get_blob(self, (void**)&node);
  if (success == -1) {
    return vv_null;
  }

  size_t len = xmlStrlen(node->name);
  char* str = malloc(xmlStrlen(node->name) + 3);
  str[0] = '<';
  str[len + 1] = '>';
  str[len + 2] = '\0';
  memcpy(str + 1, node->name, len);
  vv_object_t str_obj = vv_new_string(str);
  free(str);
  return str_obj;
}

static xml_c_node_list*
xml_node_list_new()
{
  xml_c_node_list* list = malloc(sizeof(xml_c_node_list));
  list->nodes = NULL;
  list->size = list->capacity = 0;
  return list;
}

static void
xml_node_list_append(xml_c_node_list* list, xmlNode* node)
{
  if (list->size == list->capacity) {
    if (list->size == 0) {
      list->nodes = malloc(sizeof(xmlNode*) * 8);
      list->capacity = 8;
    }
    else {
      xmlNode** new_list = malloc(sizeof(xmlNode*) * (list->size * 2));
      memcpy(new_list, list->nodes, sizeof(xmlNode*) * list->size);
      free(list->nodes);
      list->nodes = new_list;
      list->capacity *= 2;
    }
  }
  list->nodes[list->size++] = node;
}

static void
xml_node_list_free(xml_c_node_list* list)
{
  free(list->nodes);
  free(list);
}

static vv_object_t
xml_node_list_ctor(void)
{
  xml_c_node_list* list = xml_node_list_new();
  return vv_alloc_blob(list, xml_node_list_dtor);
}

static void
xml_node_list_dtor(void* blob)
{
  xml_c_node_list* node_list = blob;
  xml_node_list_free(node_list);
}

static vv_object_t
xml_node_list_init(vv_object_t self)
{
  vv_object_t arg0 = vv_get_arg(0);
  vv_object_t arg1 = vv_get_arg(1);
  if (!arg0 || !arg1) {
    return vv_null;
  }
  const char* name = xml_get_str(arg1);
  if (!name) {
    return vv_null;
  }

  if (vv_get_type(arg0) != xml_type_node) {
    return vv_null;
  }
  xmlNode* node;
  int node_success = vv_get_blob(arg0, (void**)&node);
  if (node_success == -1) {
    return vv_null;
  }
  vv_object_t success = vv_set_mem(self, vv_make_symbol("_doc"), arg0);
  if (!success) {
    return vv_null;
  }

  xml_c_node_list* list;
  int list_success = vv_get_blob(self, (void**)&list);
  if (list_success == -1) {
    return vv_null;
  }

  const xmlChar* xml_str = (const xmlChar*) name;

  for (xmlNode* i = node->children; i; i = i->next) {
    if (xmlStrcmp(i->name, xml_str) == 0) {
      xml_node_list_append(list, i);
    }
  }
  return self;
}

static vv_object_t
xml_node_list_size(vv_object_t self)
{
  xml_c_node_list* list;
  int list_success = vv_get_blob(self, (void**)&list);
  if (list_success == -1) {
    return vv_null;
  }

  return vv_new_int(list->size);
}

static vv_object_t
xml_node_list_at(vv_object_t self, vv_object_t index)
{
  int idx;
  int success = vv_get_int(index, &idx);
  if (success == -1) {
    return vv_null;
  }

  xml_c_node_list* list;
  int list_success = vv_get_blob(self, (void**)&list);

  if (list_success == -1 || idx < 0 || (size_t)idx >= list->size) {
    return vv_null;
  }

  vv_object_t doc = vv_get_mem(self, vv_make_symbol("_doc"));
  if (!doc) {
    return vv_null;
  }

  return xml_node_from(list->nodes[idx], doc);
}

static vv_object_t
xml_node_list_start(vv_object_t self)
{
  xml_c_node_list* list;
  int list_success = vv_get_blob(self, (void**)&list);
  if (list_success == -1) {
    return vv_null;
  }

  return xml_node_list_iter_from(list, 0, self);
}

static vv_object_t
xml_node_list_stop(vv_object_t self)
{
  xml_c_node_list* list;
  int list_success = vv_get_blob(self, (void**)&list);
  if (list_success == -1) {
    return vv_null;
  }

  return xml_node_list_iter_from(list, list->size, self);
}

static vv_object_t
xml_node_list_iter_from(xml_c_node_list* list,
                        size_t idx,
                        vv_object_t node_list)
{
  xml_c_node_list_iter* iter = malloc(sizeof(xml_c_node_list_iter));
  iter->list = list;
  iter->position = idx;

  vv_object_t obj = vv_new_blob(iter,
                                 xml_node_list_iter_dtor,
                                 xml_type_node_list_iterator);
  if (!obj) {
    return vv_null;
  }
  vv_object_t success = vv_set_mem(obj,
                                    vv_make_symbol("_node_list"),
                                    node_list);
  if (!success) {
    return vv_null;
  }
  return obj;
}

static vv_object_t
xml_node_list_iter_ctor(void)
{
  return vv_null;
}

static void
xml_node_list_iter_dtor(void* blob)
{
  xml_c_node_list_iter* iter = blob;
  free(iter);
}

static vv_object_t
xml_node_list_iter_get(vv_object_t self)
{
  xml_c_node_list_iter* iter;
  int success = vv_get_blob(self, (void**)&iter);
  if (success == -1) {
    return vv_null;
  }

  if (iter->list->size >= iter->position) {
    return vv_null;
  }

  vv_object_t list_obj = vv_get_mem(self, vv_make_symbol("_node_list"));
  if (!list_obj) {
    return vv_null;
  }
  vv_object_t doc_obj = vv_get_mem(list_obj, vv_make_symbol("_doc"));
  if (!doc_obj) {
    return vv_null;
  }

  return xml_node_from(iter->list->nodes[iter->position], doc_obj);
}

static vv_object_t
xml_node_list_iter_equals(vv_object_t self, vv_object_t other)
{
  if (vv_get_type(other) != xml_type_node_list_iterator) {
    return vv_null;
  }

  xml_c_node_list_iter* iter;
  int success = vv_get_blob(self, (void**)&iter);
  if (success == -1) {
    return vv_null;
  }

  xml_c_node_list_iter* other_iter;
  int other_succ = vv_get_blob(other, (void**)&other_iter);
  if (other_succ == -1) {
    return vv_null;
  }

  if (iter->list != other_iter->list) {
    return vv_null;
  }

  return vv_new_bool(iter->position == other_iter->position);
}

static vv_object_t
xml_node_list_iter_unequal(vv_object_t self, vv_object_t other)
{
  if (vv_get_type(other) != xml_type_node_list_iterator) {
    return vv_null;
  }

  xml_c_node_list_iter* iter;
  int success = vv_get_blob(self, (void**)&iter);
  if (success == -1) {
    return vv_null;
  }

  xml_c_node_list_iter* other_iter;
  int other_succ = vv_get_blob(other, (void**)&other_iter);
  if (other_succ == -1) {
    return vv_null;
  }

  if (iter->list != other_iter->list) {
    return vv_null;
  }

  return vv_new_bool(iter->position != other_iter->position);
}

static vv_object_t
xml_node_list_iter_increment(vv_object_t self)
{
  xml_c_node_list_iter* iter;
  int success = vv_get_blob(self, (void**)&iter);
  if (success == -1) {
    return vv_null;
  }

  if (iter->position == iter->list->size) {
    return vv_null;
  }

  ++iter->position;
  return self;
}

__attribute__((destructor))
static void
xml_cleanup(void)
{
  xmlCleanupParser();
}

void
vv_init_lib(void)
{
  xml_type_document = vv_new_type("XMLDocument",
                                  vv_null,
                                  xml_document_ctor);
  if (xml_type_document) {
    vv_add_binop(xml_type_document, "init", xml_document_init);
    vv_add_monop(xml_type_document, "root", xml_document_root);
    vv_add_monop(xml_type_document, "to_str", xml_document_to_str);
  }

  xml_type_node = vv_new_type("XMLNode",
                              vv_null,
                              xml_node_ctor);
  if (xml_type_node) {
    vv_add_binop(xml_type_node, "all", xml_node_all);
    vv_add_binop(xml_type_node, "first", xml_node_first);
    vv_add_binop(xml_type_node, "attr", xml_node_attr);
    vv_add_monop(xml_type_node, "name", xml_node_name);
    vv_add_monop(xml_type_node, "contents", xml_node_contents);
    vv_add_monop(xml_type_node, "str", xml_node_str);
  }

  xml_type_node_list = vv_new_type("XMLNodeList",
                                   vv_null,
                                   xml_node_list_ctor);
  if (xml_type_node_list) {
    vv_add_method(xml_type_node_list, "init", xml_node_list_init, 2);
    vv_add_monop(xml_type_node_list, "size", xml_node_list_size);
    vv_add_binop(xml_type_node_list, "at", xml_node_list_at);
    vv_add_monop(xml_type_node_list, "start", xml_node_list_start);
    vv_add_monop(xml_type_node_list, "stop", xml_node_list_stop);
  }

  xml_type_node_list_iterator = vv_new_type("XMLNodeListIterator",
                                            vv_null,
                                            xml_node_list_iter_ctor);
  if (xml_type_node_list_iterator) {
    vv_add_monop(xml_type_node_list_iterator, "get", xml_node_list_iter_get);
    vv_add_binop(xml_type_node_list_iterator,
                 "equals",
                 xml_node_list_iter_equals);
    vv_add_binop(xml_type_node_list_iterator,
                 "unequal",
                 xml_node_list_iter_unequal);
    vv_add_monop(xml_type_node_list_iterator,
                 "increment",
                 xml_node_list_iter_increment);
  }
}
