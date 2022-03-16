#include <stdint.h>
#include <string.h>
#include <errno.h>
#if 1
#include <stdio.h> /* popen/pclose */
#else
#include "secure_wrapper.h"
#endif

#define ARC_BOARD_CMD_GET "arc-board get %s"
#define ARC_BOARD_CMD_SET "arc-board set %s %s"

typedef struct mapping {
  char *fd_id;
  char *arc_fd_id;
  uint8_t writable;
} factoryMappingItem;

factoryMappingItem factoryMappingTable[] = {
  {"base-mac", "basemac", 0},
  {"model", "model", 0},
  {"partner-id", "partner_id", 1},
  {"client-cert", "client_cert", 1},
  {"client-priv-key", "client_key", 1},
  {"ssid", "wifi-ssid", 1},
  {"ssid-pass", "wifi-pwd", 1},
  {"boot-pass", "boot_pwd", 0},
  {"root-pass", "root_pwd", 0},
  {"device-pass", "web_pwd", 1},
  {"device-seed", "device_seed", 0},
  {"serial", "sn", 0},
  {"onu-serial", NULL, 0},
  {"prod-date", "prod-date", 0},
  {"hw-rev", "hwver", 0},
  {"manuf", "manuf", 0},
  {"oui", "oui", 0},
  {"annex-id", "hwtype", 0},
  {NULL, NULL, 0}
};

static factoryMappingItem * findMapping(const char *id)
{
  int i = 0;
  int iBoardDataItems = (int)sizeof(factoryMappingTable)/sizeof(factoryMappingItem);
  factoryMappingItem *item = NULL;
  
  for(i = 0; i < iBoardDataItems; i++)
  {
    item = &factoryMappingTable[i];
    if(strcmp(id, item->fd_id)==0)
    {
      return item;
    }
  }
  if(item == NULL || item->fd_id == NULL || item->arc_fd_id == NULL)
  {
    fprintf(stderr, "error: %s: fd_id %s not found!!\n", __FUNCTION__, id);
    return NULL;
  }
  return item;
}

/* Retrieve a factory-data entry.
 *
 * In case the element is stored encrypted it will be
 * decrypted before being returned.
 *
 * This function allocates memory which must be free'd
 * by its caller.
 *
 * On error no memory must be free'd.
 *
 * All returned data is guaranteed to be '\0' terminated.
 * The sz parameter indicates the real size of the entry without
 * '\0' termination.
 *
 * @param id      The id of the factory-data entry to retrieve.
 * @param dest    Pointer to a pointer that will be set to the
 *                memory address where the entry is stored.
 * @param sz      Pointer to the size of the entry (can be NULL
 *                for string entries).
 *
 * @return 0 on success,
 * 	-EINVAL on erroneous parameters,
 * 	-ENOENT if `id` is not know,
 * 	-ENOMEM if allocating of memory failed,
 */
int rdkf_fd_get(const char *id, void **dest, size_t *sz)
{
  char cmd[128] = {'\0'};
  char result[128] = {'\0'};
  factoryMappingItem *item = NULL;
  FILE *fp;
  int i;
  //char *result = NULL;

  /* check id mapping */
  item = findMapping(id);
  if(item == NULL)
    return -ENOENT;

  /* commpose command */
#if 1
  snprintf(cmd, sizeof(cmd),  ARC_BOARD_CMD_GET, item->arc_fd_id);
  if((fp = v_secure_popen("r", cmd)) != NULL)
#else
  if((fp = v_secure_popen("r", ARC_BOARD_CMD_GET, item->arc_fd_id)) != NULL)
#endif
  {
    fgets(result, sizeof(result)-1, fp);
#if 1
    pclose(fp);
#else
    v_secure_pclose(fp);
#endif
  }

  for(i=0; i < (int)sizeof(result); i++)
  {
    if(result[i] == '\0')
      break;
    if( (i == (sizeof(result) - 1)) ||
        (result[i] == '\r') ||
        (result[i] == '\n') )
    {
      result[i] = '\0';
      break;
    }
  }

  if(strlen(result) > 0)
  {
    *dest = strdup(result);
    if(*dest == NULL)
      return -ENOMEM;

    *sz = strlen(result);
  return 0;
}

  return -EINVAL;
}

/* Update a factory-data entry.
 *
 * In case an element has to be stored encrypted, this function
 * will take care of it.
 *
 * @param id      The id of the factory-data entry to update.
 * @param src     Pointer to the entry.
 * @param sz      Size of the entry.
 *
 * @return 0 on success
 * 	-EINVAL on erroneous parameters,
 * 	-ENOENT if `id` is not known,
 * 	-EACCES if `id` is not writable,
 * 	-ENOMEM if writing the `id` failed,
 * 	Any other negative value to indicate other failures
 */
int rdkf_fd_set(const char *id, const void *src, size_t sz)
{
  char cmd[128] = {'\0'};
  char result[128] = {'\0'};
  factoryMappingItem *item = NULL;
  FILE *fp;
  int i;
  //char *result = NULL;

  if(src==NULL || sz <= 0)
    return -EINVAL;

  /* check id mapping */
  item = findMapping(id);
  if(!item || !item->fd_id || !item->arc_fd_id)
    return -ENOENT;

  if(item->writable == 0)
    return -EACCES;

  /* check sz too large that overwiitten to cmd buffer */
  i = snprintf(NULL, 0, ARC_BOARD_CMD_SET, item->arc_fd_id, (const char *)src);
  if(i > (int)(sizeof(cmd) - 1))
    return -ENOMEM;

#if 1
  snprintf(cmd, sizeof(cmd), ARC_BOARD_CMD_SET, item->arc_fd_id, (const char *)src);
  if((fp = popen(cmd, "r")) != NULL)
#else
  if ((fp = v_secure_popen("r", ARC_BOARD_CMD_SET, item->arc_fd_id, (const char *)src))!= NULL)
#endif
  {
    fgets(result, sizeof(result)-1, fp);
#if 1
    pclose(fp);
#else
    v_secure_pclose(fp);
#endif
  }

  if(strlen(result) > 0)
  {
    if(strstr(result, "ERROR") != NULL)
      return -EINVAL;
  }
  
  return 0;
}

