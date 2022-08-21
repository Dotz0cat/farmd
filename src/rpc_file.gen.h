
/*
 * Automatically generated from src/rpc_file.rpc
 */

#ifndef EVENT_RPCOUT_SRC_RPC_FILE_RPC_
#define EVENT_RPCOUT_SRC_RPC_FILE_RPC_


#include <event2/util.h> /* for ev_uint*_t */
#include <event2/rpc.h>
struct BarnQueryRequest;
struct BarnQueryReply;
struct SiloQueryRequest;
struct SiloQueryReply;

/* Tag definition for BarnQueryRequest */
enum barnqueryrequest_ {
  BARNQUERYREQUEST_ITEM_NAME=1,
  BARNQUERYREQUEST_MAX_TAGS
};

/* Structure declaration for BarnQueryRequest */
struct BarnQueryRequest_access_ {
  int (*item_name_assign)(struct BarnQueryRequest *, const char *);
  int (*item_name_get)(struct BarnQueryRequest *, char * *);
};

struct BarnQueryRequest {
  struct BarnQueryRequest_access_ *base;

  char *item_name_data;

  ev_uint8_t item_name_set;
};

struct BarnQueryRequest *BarnQueryRequest_new(void);
struct BarnQueryRequest *BarnQueryRequest_new_with_arg(void *);
void BarnQueryRequest_free(struct BarnQueryRequest *);
void BarnQueryRequest_clear(struct BarnQueryRequest *);
void BarnQueryRequest_marshal(struct evbuffer *, const struct BarnQueryRequest *);
int BarnQueryRequest_unmarshal(struct BarnQueryRequest *, struct evbuffer *);
int BarnQueryRequest_complete(struct BarnQueryRequest *);
void evtag_marshal_BarnQueryRequest(struct evbuffer *, ev_uint32_t,
    const struct BarnQueryRequest *);
int evtag_unmarshal_BarnQueryRequest(struct evbuffer *, ev_uint32_t,
    struct BarnQueryRequest *);
int BarnQueryRequest_item_name_assign(struct BarnQueryRequest *, const char *);
int BarnQueryRequest_item_name_get(struct BarnQueryRequest *, char * *);
/* --- BarnQueryRequest done --- */

/* Tag definition for BarnQueryReply */
enum barnqueryreply_ {
  BARNQUERYREPLY_ITEMS=1,
  BARNQUERYREPLY_MAX_TAGS
};

/* Structure declaration for BarnQueryReply */
struct BarnQueryReply_access_ {
  int (*items_assign)(struct BarnQueryReply *, const ev_uint32_t);
  int (*items_get)(struct BarnQueryReply *, ev_uint32_t *);
};

struct BarnQueryReply {
  struct BarnQueryReply_access_ *base;

  ev_uint32_t items_data;

  ev_uint8_t items_set;
};

struct BarnQueryReply *BarnQueryReply_new(void);
struct BarnQueryReply *BarnQueryReply_new_with_arg(void *);
void BarnQueryReply_free(struct BarnQueryReply *);
void BarnQueryReply_clear(struct BarnQueryReply *);
void BarnQueryReply_marshal(struct evbuffer *, const struct BarnQueryReply *);
int BarnQueryReply_unmarshal(struct BarnQueryReply *, struct evbuffer *);
int BarnQueryReply_complete(struct BarnQueryReply *);
void evtag_marshal_BarnQueryReply(struct evbuffer *, ev_uint32_t,
    const struct BarnQueryReply *);
int evtag_unmarshal_BarnQueryReply(struct evbuffer *, ev_uint32_t,
    struct BarnQueryReply *);
int BarnQueryReply_items_assign(struct BarnQueryReply *, const ev_uint32_t);
int BarnQueryReply_items_get(struct BarnQueryReply *, ev_uint32_t *);
/* --- BarnQueryReply done --- */

/* Tag definition for SiloQueryRequest */
enum siloqueryrequest_ {
  SILOQUERYREQUEST_ITEM_NAME=1,
  SILOQUERYREQUEST_MAX_TAGS
};

/* Structure declaration for SiloQueryRequest */
struct SiloQueryRequest_access_ {
  int (*item_name_assign)(struct SiloQueryRequest *, const char *);
  int (*item_name_get)(struct SiloQueryRequest *, char * *);
};

struct SiloQueryRequest {
  struct SiloQueryRequest_access_ *base;

  char *item_name_data;

  ev_uint8_t item_name_set;
};

struct SiloQueryRequest *SiloQueryRequest_new(void);
struct SiloQueryRequest *SiloQueryRequest_new_with_arg(void *);
void SiloQueryRequest_free(struct SiloQueryRequest *);
void SiloQueryRequest_clear(struct SiloQueryRequest *);
void SiloQueryRequest_marshal(struct evbuffer *, const struct SiloQueryRequest *);
int SiloQueryRequest_unmarshal(struct SiloQueryRequest *, struct evbuffer *);
int SiloQueryRequest_complete(struct SiloQueryRequest *);
void evtag_marshal_SiloQueryRequest(struct evbuffer *, ev_uint32_t,
    const struct SiloQueryRequest *);
int evtag_unmarshal_SiloQueryRequest(struct evbuffer *, ev_uint32_t,
    struct SiloQueryRequest *);
int SiloQueryRequest_item_name_assign(struct SiloQueryRequest *, const char *);
int SiloQueryRequest_item_name_get(struct SiloQueryRequest *, char * *);
/* --- SiloQueryRequest done --- */

/* Tag definition for SiloQueryReply */
enum siloqueryreply_ {
  SILOQUERYREPLY_ITEMS=1,
  SILOQUERYREPLY_MAX_TAGS
};

/* Structure declaration for SiloQueryReply */
struct SiloQueryReply_access_ {
  int (*items_assign)(struct SiloQueryReply *, const ev_uint32_t);
  int (*items_get)(struct SiloQueryReply *, ev_uint32_t *);
};

struct SiloQueryReply {
  struct SiloQueryReply_access_ *base;

  ev_uint32_t items_data;

  ev_uint8_t items_set;
};

struct SiloQueryReply *SiloQueryReply_new(void);
struct SiloQueryReply *SiloQueryReply_new_with_arg(void *);
void SiloQueryReply_free(struct SiloQueryReply *);
void SiloQueryReply_clear(struct SiloQueryReply *);
void SiloQueryReply_marshal(struct evbuffer *, const struct SiloQueryReply *);
int SiloQueryReply_unmarshal(struct SiloQueryReply *, struct evbuffer *);
int SiloQueryReply_complete(struct SiloQueryReply *);
void evtag_marshal_SiloQueryReply(struct evbuffer *, ev_uint32_t,
    const struct SiloQueryReply *);
int evtag_unmarshal_SiloQueryReply(struct evbuffer *, ev_uint32_t,
    struct SiloQueryReply *);
int SiloQueryReply_items_assign(struct SiloQueryReply *, const ev_uint32_t);
int SiloQueryReply_items_get(struct SiloQueryReply *, ev_uint32_t *);
/* --- SiloQueryReply done --- */

#endif  /* EVENT_RPCOUT_SRC_RPC_FILE_RPC_ */