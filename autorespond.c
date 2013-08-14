#define AUTORESPOND "autorespond"
#define PLUGIN_DIR ".purple/plugins"
#define PLUGIN_LUA "autorespond.lua"
#define PROTOCOL_PREFIX "prpl-"

#include <string.h>

//Purple plugin
#define PURPLE_PLUGINS
#include <libpurple/debug.h>
#include <libpurple/version.h>
#include <libpurple/conversation.h>
#include <libpurple/plugin.h>
#include <libpurple/savedstatuses.h>
#include <libpurple/signals.h>
#include <libpurple/util.h>

//Lua Libraries
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h> 

char *message = NULL;
char *hook_script = NULL;

typedef struct {
    const char *text;
    const char *protocol_id;
    const char *remote_from_group;
    const char *remote_username;
    const char *remote_alias;
    const char *remote_status_id;
    const char *remote_status_msg;
    const char *local_username;
    const char *local_alias;
    const char *local_status_id;
    const char *local_status_msg;
} msg_metadata_t;

void get_plugin_dir(char *buf)
{
    sprintf(buf, "%s/%s", g_get_home_dir(), PLUGIN_DIR);
}

void get_plugin_lua(char *buf)
{
    get_plugin_dir(buf);
    sprintf(buf, "%s/%s", buf, PLUGIN_LUA);
}

void push_val_to_lua_t(lua_State *L, const char *key, const char *val)
{
    lua_pushstring(L, key);   
    lua_pushstring(L, val);
    lua_settable(L, -3); 
}

void run_lua(PurpleConversation *conv, msg_metadata_t msg)
{
    lua_State *L;

    L = luaL_newstate(); 
    luaL_openlibs(L);  
    char lua_path[50];
    get_plugin_lua(lua_path);

    if (luaL_loadfile(L, lua_path))
    {
        lua_close(L);
        return;
    }

    if (lua_pcall(L, 0, 0, 0))  
    {
        lua_close(L);
        return;
    }

    lua_getglobal(L, "autorespond");
    lua_newtable(L); 

    push_val_to_lua_t(L, "text", msg.text);
    push_val_to_lua_t(L, "protocol_id", msg.protocol_id);
    push_val_to_lua_t(L, "remote_from_group", msg.remote_from_group);
    push_val_to_lua_t(L, "remote_username", msg.remote_username);
    push_val_to_lua_t(L, "remote_alias", msg.remote_alias);
    push_val_to_lua_t(L, "remote_status_id", msg.remote_status_id);
    push_val_to_lua_t(L, "remote_status_msg", msg.remote_status_msg);
    push_val_to_lua_t(L, "local_username", msg.local_username);
    push_val_to_lua_t(L, "local_alias", msg.local_alias);
    push_val_to_lua_t(L, "local_status_id", msg.local_status_id);
    push_val_to_lua_t(L, "local_status_msg", msg.local_status_msg);

    //lua_pushstring(L, msg.text);
    if (lua_pcall(L, 1, 1, 0))                  /* Run function, !!! NRETURN=1 !!! */
    {
        lua_close(L);
        return;
    }
    const char* resp = lua_tostring(L, -1);
    if(resp)
    {
        purple_conv_im_send(purple_conversation_get_im_data(conv), resp);
    }
    lua_close(L); 
}

static void received_im_msg_cb(PurpleAccount *account, char *who, char *buffer, PurpleConversation *conv, PurpleMessageFlags flags, void *data) {
    if (conv == NULL) conv = purple_conversation_new(PURPLE_CONV_TYPE_IM, account, who); //* A workaround to avoid skipping of the first message as a result on NULL-conv: */
    PurpleBuddy *buddy = purple_find_buddy(account, who);
    PurplePresence *presence = purple_buddy_get_presence(buddy);
    msg_metadata_t msg;

    //Get message
    msg.text = purple_markup_strip_html(buffer);
    msg.remote_username = who;

    //LOCAL USER:
    msg.local_alias = purple_account_get_alias(account);
    msg.local_username = (char *) purple_account_get_name_for_display(account);

    //REMOTE USER (Buddy):
    //Get buddy alias
    msg.remote_alias = purple_buddy_get_alias(buddy);
    if(msg.remote_alias == NULL) msg.remote_alias = "";

    //Get buddy group
    PurpleGroup *group = purple_buddy_get_group(buddy);
    msg.remote_from_group = group != NULL ? purple_group_get_name(group) : ""; //return empty string if not in group

    //Get protocol ID
    msg.protocol_id = purple_account_get_protocol_id(account);
    if(!strncmp(msg.protocol_id,PROTOCOL_PREFIX,strlen(PROTOCOL_PREFIX))) msg.protocol_id += strlen(PROTOCOL_PREFIX); //trim out PROTOCOL_PREFIX (eg.: "prpl-irc" => "irc")

    //Get status
    PurpleStatus *status = purple_account_get_active_status(account);
    PurpleStatusType *type = purple_status_get_type(status);
    //remote
    PurpleStatus *r_status = purple_presence_get_active_status(presence);
    PurpleStatusType *r_status_type =	purple_status_get_type(r_status);

    //Get status id
    msg.local_status_id = NULL;
    msg.local_status_id = purple_primitive_get_id_from_type(purple_status_type_get_primitive(type));
    //remote
    msg.remote_status_id = NULL;
    msg.remote_status_id = purple_primitive_get_id_from_type(purple_status_type_get_primitive(r_status_type));

    //Get status message
    msg.local_status_msg = NULL;
    if (purple_status_type_get_attr(type, "message") != NULL) {
        msg.local_status_msg = purple_status_get_attr_string(status, "message");
    } else {
        PurpleSavedStatus *savedstatus = purple_savedstatus_get_current();
        if(savedstatus)
            msg.local_status_msg = purple_savedstatus_get_message(savedstatus);
    }
    //remote
    msg.remote_status_msg = NULL;
    if (purple_status_type_get_attr(r_status_type, "message") != NULL) {
        msg.remote_status_msg = purple_status_get_attr_string(r_status, "message");
    } else {
        msg.remote_status_msg = "";
    }

    run_lua(conv, msg);
}

static gboolean plugin_load(PurplePlugin * plugin) {
    asprintf(&hook_script,"%s/%s",purple_user_dir(),AUTORESPOND);
    void *conv_handle = purple_conversations_get_handle();
    purple_signal_connect(conv_handle, "received-im-msg", plugin, PURPLE_CALLBACK(received_im_msg_cb), NULL);
    return TRUE;
}

static gboolean plugin_unload(PurplePlugin * plugin) {
    free(hook_script);
    return TRUE;
}

static PurplePluginInfo info = {
    PURPLE_PLUGIN_MAGIC,
    PURPLE_MAJOR_VERSION,
    PURPLE_MINOR_VERSION,
    PURPLE_PLUGIN_STANDARD,
    NULL,
    0,
    NULL,
    PURPLE_PRIORITY_DEFAULT,

    "core-autorespond",
    "Autorespond",
    "0.1.0",
    "Framework for hooking scripts to process received messages for libpurple clients",
    "\nThis plugin will execute the lua script in \"~/.purple/" PLUGIN_LUA "\" "
        " and respond back with the string returned from the function autorespond(msg).\n"
        " Inspired by the answerscripts plugin for pidgin.\n"
        "\n",
    "Bret Lowrey",
    "http://github.com/lowrey/libpurple_autorespond",

    plugin_load,
    plugin_unload,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

static void init_plugin(PurplePlugin * plugin) {
}

PURPLE_INIT_PLUGIN(autorespond, init_plugin, info)

