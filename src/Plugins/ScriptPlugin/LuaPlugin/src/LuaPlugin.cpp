/**
 * @file LuaPlugin.cpp
 * @brief Implementation of the LuaPlugin class
 */

#include "LuaPlugin.h"
#include "PluginExport.h"
#include "MathPlugin.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <rtm/types.h>

// Use Vector3 from MathPlugin
using Vector3 = rtm::vector4f;

// Include Lua headers
extern "C" {
    #include <lua.h>
    #include <lauxlib.h>
    #include <lualib.h>
}

// Define plugin info
PluginInfo LuaPlugin::pluginInfo_("LuaPlugin",                // name
                                "Lua Script Plugin",        // displayName
                                "Provides Lua scripting integration", // description
                                PluginInfo::Version(1, 0, 0),  // version
                                "Plugin System Team");       // author

// Add dependencies
static bool initPluginInfo = []() {
    LuaPlugin::pluginInfo_.AddDependency({"ScriptPlugin", {0, 1, 0}, false}); // Depends on ScriptPlugin
    LuaPlugin::pluginInfo_.AddDependency({"MathPlugin", {0, 1, 0}, false});   // Depends on MathPlugin for math operations
    return true;
}();

// Helper function to check if a file exists
static bool FileExists(const std::string& filePath) {
    std::ifstream file(filePath);
    return file.good();
}

// Helper function to read a file into a string
static std::string ReadFileToString(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return "";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// Vector3 wrapper for Lua
static const char* VECTOR3_METATABLE = "Vector3";

// Constructor for Vector3 in Lua
static int Vector3_New(lua_State* L) {
    float x = static_cast<float>(luaL_optnumber(L, 1, 0.0));
    float y = static_cast<float>(luaL_optnumber(L, 2, 0.0));
    float z = static_cast<float>(luaL_optnumber(L, 3, 0.0));
    
    // Create a new Vector3 userdata
auto* vec = static_cast<Vector3*>(lua_newuserdata(L, sizeof(Vector3)));
*vec = MathPlugin::CreateVector3(x, y, z);
    
    // Set the metatable for the userdata
    luaL_getmetatable(L, VECTOR3_METATABLE);
    lua_setmetatable(L, -2);
    
    return 1;
}

// Get Vector3 from Lua stack
static Vector3* CheckVector3(lua_State* L, int index) {
    void* userData = luaL_checkudata(L, index, VECTOR3_METATABLE);
    luaL_argcheck(L, userData != nullptr, index, "Vector3 expected");
    return static_cast<Vector3*>(userData);
}

// Vector3 addition
static int Vector3_Add(lua_State* L) {
    Vector3* a = CheckVector3(L, 1);
    Vector3* b = CheckVector3(L, 2);
    
    auto* result = static_cast<Vector3*>(lua_newuserdata(L, sizeof(Vector3)));
    *result = MathPlugin::Vector3Add(*a, *b);
    
    luaL_getmetatable(L, VECTOR3_METATABLE);
    lua_setmetatable(L, -2);
    
    return 1;
}

// Vector3 subtraction
static int Vector3_Sub(lua_State* L) {
    Vector3* a = CheckVector3(L, 1);
    Vector3* b = CheckVector3(L, 2);
    
    auto* result = static_cast<Vector3*>(lua_newuserdata(L, sizeof(Vector3)));
    *result = MathPlugin::Vector3Subtract(*a, *b);
    
    luaL_getmetatable(L, VECTOR3_METATABLE);
    lua_setmetatable(L, -2);
    
    return 1;
}

// Vector3 dot product
static int Vector3_Dot(lua_State* L) {
    Vector3* a = CheckVector3(L, 1);
    Vector3* b = CheckVector3(L, 2);
    
    float dot = MathPlugin::Vector3Dot(*a, *b);
    lua_pushnumber(L, dot);
    
    return 1;
}

// Vector3 cross product
static int Vector3_Cross(lua_State* L) {
    Vector3* a = CheckVector3(L, 1);
    Vector3* b = CheckVector3(L, 2);
    
    auto* result = static_cast<Vector3*>(lua_newuserdata(L, sizeof(Vector3)));
    *result = MathPlugin::Vector3Cross(*a, *b);
    
    luaL_getmetatable(L, VECTOR3_METATABLE);
    lua_setmetatable(L, -2);
    
    return 1;
}

// Vector3 length
static int Vector3_Length(lua_State* L) {
    Vector3* vec = CheckVector3(L, 1);
    float length = MathPlugin::Vector3Length(*vec);
    lua_pushnumber(L, length);
    return 1;
}

// Vector3 normalize
static int Vector3_Normalize(lua_State* L) {
    Vector3* vec = CheckVector3(L, 1);
    
    auto* result = static_cast<Vector3*>(lua_newuserdata(L, sizeof(Vector3)));
    *result = MathPlugin::Vector3Normalize(*vec);
    
    luaL_getmetatable(L, VECTOR3_METATABLE);
    lua_setmetatable(L, -2);
    
    return 1;
}

// Vector3 tostring
static int Vector3_ToString(lua_State* L) {
    Vector3* vec = CheckVector3(L, 1);
    
    std::stringstream ss;
    float x, y, z;
    MathPlugin::GetVector3Components(*vec, x, y, z);
    ss << "Vector3(" << x << ", " << y << ", " << z << ")";
    lua_pushstring(L, ss.str().c_str());
    return 1;
}

// Vector3 index (get component)
static int Vector3_Index(lua_State* L) {
    Vector3* vec = CheckVector3(L, 1);
    const char* key = luaL_checkstring(L, 2);
    
    float x, y, z;
    MathPlugin::GetVector3Components(*vec, x, y, z);
    
    if (strcmp(key, "x") == 0) {
        lua_pushnumber(L, x);
    } else if (strcmp(key, "y") == 0) {
        lua_pushnumber(L, y);
    } else if (strcmp(key, "z") == 0) {
        lua_pushnumber(L, z);
    } else {
        // Check the metatable for methods
        luaL_getmetatable(L, VECTOR3_METATABLE);
        lua_pushstring(L, key);
        lua_rawget(L, -2);
        if (lua_isnil(L, -1)) {
            lua_pop(L, 2);
            luaL_error(L, "Invalid Vector3 component or method: %s", key);
            return 0;
        }
    }
    
    return 1;
}

// Vector3 newindex (set component)
static int Vector3_NewIndex(lua_State* L) {
    Vector3* vec = CheckVector3(L, 1);
    const char* key = luaL_checkstring(L, 2);
    float value = static_cast<float>(luaL_checknumber(L, 3));
    
    float x, y, z;
    MathPlugin::GetVector3Components(*vec, x, y, z);
    
    if (strcmp(key, "x") == 0) {
        x = value;
    } else if (strcmp(key, "y") == 0) {
        y = value;
    } else if (strcmp(key, "z") == 0) {
        z = value;
    } else {
        luaL_error(L, "Cannot set invalid Vector3 component: %s", key);
    }
    
    *vec = MathPlugin::CreateVector3(x, y, z);
    
    return 0;
}

// Register Vector3 type with Lua
static void RegisterVector3(lua_State* L) {
    // Create metatable for Vector3
    luaL_newmetatable(L, VECTOR3_METATABLE);
    
    // Metatable.__index = metatable (for methods)
    lua_pushstring(L, "__index");
    lua_pushcfunction(L, Vector3_Index);
    lua_settable(L, -3);
    
    // Metatable.__newindex
    lua_pushstring(L, "__newindex");
    lua_pushcfunction(L, Vector3_NewIndex);
    lua_settable(L, -3);
    
    // Metatable.__add
    lua_pushstring(L, "__add");
    lua_pushcfunction(L, Vector3_Add);
    lua_settable(L, -3);
    
    // Metatable.__sub
    lua_pushstring(L, "__sub");
    lua_pushcfunction(L, Vector3_Sub);
    lua_settable(L, -3);
    
    // Metatable.__tostring
    lua_pushstring(L, "__tostring");
    lua_pushcfunction(L, Vector3_ToString);
    lua_settable(L, -3);
    
    // Add methods to the metatable
    lua_pushstring(L, "dot");
    lua_pushcfunction(L, Vector3_Dot);
    lua_settable(L, -3);
    
    lua_pushstring(L, "cross");
    lua_pushcfunction(L, Vector3_Cross);
    lua_settable(L, -3);
    
    lua_pushstring(L, "length");
    lua_pushcfunction(L, Vector3_Length);
    lua_settable(L, -3);
    
    lua_pushstring(L, "normalize");
    lua_pushcfunction(L, Vector3_Normalize);
    lua_settable(L, -3);
    
    // Pop the metatable
    lua_pop(L, 1);
    
    // Create a global Vector3 constructor
    lua_pushcfunction(L, Vector3_New);
    lua_setglobal(L, "Vector3");
}

// Constructor
LuaPlugin::LuaPlugin()
    : luaState_(nullptr)
    , initialized_(false) {
}

// Destructor
LuaPlugin::~LuaPlugin() {
    if (initialized_) {
        Shutdown();
    }
}

// Initialize the plugin
bool LuaPlugin::Initialize() {
    if (initialized_) {
        return true;
    }
    
    // Create a new Lua state
    luaState_ = luaL_newstate();
    if (!luaState_) {
        return false;
    }
    
    // Open standard libraries
    luaL_openlibs(luaState_);
    
    // Register built-in functions
    if (!RegisterBuiltins()) {
        lua_close(luaState_);
        luaState_ = nullptr;
        return false;
    }
    
    // Register math functions
    if (!RegisterMathFunctions()) {
        lua_close(luaState_);
        luaState_ = nullptr;
        return false;
    }
    
    initialized_ = true;
    return true;
}

// Shutdown the plugin
void LuaPlugin::Shutdown() {
    if (!initialized_) {
        return;
    }
    
    // Close Lua state
    if (luaState_) {
        lua_close(luaState_);
        luaState_ = nullptr;
    }
    
    initialized_ = false;
}

// Get plugin information
const PluginInfo& LuaPlugin::GetPluginInfo() const {
    return pluginInfo_;
}

// Serialize plugin state
std::string LuaPlugin::Serialize() {
    // In a real implementation, you would serialize the Lua state
    return "";
}

// Deserialize plugin state
bool LuaPlugin::Deserialize(const std::string& data) {
    // In a real implementation, you would deserialize the Lua state
    return true;
}

// Prepare for hot reload
bool LuaPlugin::PrepareForHotReload() {
    // In a real implementation, you would prepare the Lua state for hot reload
    return true;
}

// Complete hot reload
bool LuaPlugin::CompleteHotReload() {
    // In a real implementation, you would complete the hot reload of the Lua state
    return true;
}

// Execute a Lua script file
bool LuaPlugin::ExecuteFile(const std::string& filePath) {
    if (!initialized_ || !luaState_) {
        return false;
    }
    
    // Check if file exists
    if (!FileExists(filePath)) {
        return false;
    }
    
    // Read file content
    std::string script = ReadFileToString(filePath);
    if (script.empty()) {
        return false;
    }
    
    // Execute script
    int result = luaL_dostring(luaState_, script.c_str());
    if (result != LUA_OK) {
        // Handle error
        const char* errorMsg = lua_tostring(luaState_, -1);
        lua_pop(luaState_, 1); // Pop error message
        return false;
    }
    
    return true;
}

// Execute a Lua script string
bool LuaPlugin::ExecuteString(const std::string& script) {
    if (!initialized_ || !luaState_) {
        return false;
    }
    
    // Execute script
    int result = luaL_dostring(luaState_, script.c_str());
    if (result != LUA_OK) {
        // Handle error
        const char* errorMsg = lua_tostring(luaState_, -1);
        lua_pop(luaState_, 1); // Pop error message
        return false;
    }
    
    return true;
}

// Evaluate a Lua expression
bool LuaPlugin::EvaluateExpression(const std::string& expression, std::string& result) {
    if (!initialized_ || !luaState_) {
        return false;
    }
    
    // Create a Lua chunk that returns the expression result
    std::string chunk = "return " + expression;
    
    // Execute chunk
    int loadResult = luaL_loadstring(luaState_, chunk.c_str());
    if (loadResult != LUA_OK) {
        // Handle load error
        const char* errorMsg = lua_tostring(luaState_, -1);
        lua_pop(luaState_, 1); // Pop error message
        return false;
    }
    
    // Call the chunk
    int callResult = lua_pcall(luaState_, 0, 1, 0);
    if (callResult != LUA_OK) {
        // Handle call error
        const char* errorMsg = lua_tostring(luaState_, -1);
        lua_pop(luaState_, 1); // Pop error message
        return false;
    }
    
    // Get result
    if (lua_isnil(luaState_, -1)) {
        result = "nil";
    } else if (lua_isboolean(luaState_, -1)) {
        result = lua_toboolean(luaState_, -1) ? "true" : "false";
    } else if (lua_isnumber(luaState_, -1)) {
        result = std::to_string(lua_tonumber(luaState_, -1));
    } else if (lua_isstring(luaState_, -1)) {
        result = lua_tostring(luaState_, -1);
    } else {
        // For other types, get the type name
        result = lua_typename(luaState_, lua_type(luaState_, -1));
    }
    
    // Pop result
    lua_pop(luaState_, 1);
    
    return true;
}

// Register a C function with Lua
bool LuaPlugin::RegisterFunction(const std::string& name, void* function) {
    if (!initialized_ || !luaState_) {
        return false;
    }
    
    // Cast function pointer to lua_CFunction
    lua_CFunction luaFunc = reinterpret_cast<lua_CFunction>(function);
    
    // Register function
    lua_pushcfunction(luaState_, luaFunc);
    lua_setglobal(luaState_, name.c_str());
    
    return true;
}

// Register a C++ object with Lua
bool LuaPlugin::RegisterObject(const std::string& name, void* object) {
    // In a real implementation, you would register the object with Lua
    return false;
}

// Get supported file extensions
std::vector<std::string> LuaPlugin::GetSupportedExtensions() const {
    return {".lua"};
}

// Get language name
std::string LuaPlugin::GetLanguageName() const {
    return "Lua";
}

// Get language version
std::string LuaPlugin::GetLanguageVersion() const {
    if (!initialized_ || !luaState_) {
        return "Unknown";
    }
    
    // Get Lua version
    lua_getglobal(luaState_, "_VERSION");
    std::string version = lua_tostring(luaState_, -1);
    lua_pop(luaState_, 1);
    
    return version;
}

// Helper function to register a C function with Lua
bool LuaPlugin::RegisterCFunction(const std::string& name, lua_CFunction function) {
    if (!initialized_ || !luaState_) {
        return false;
    }
    
    lua_pushcfunction(luaState_, function);
    lua_setglobal(luaState_, name.c_str());
    
    return true;
}

// Lua print function
static int LuaPrint(lua_State* L) {
    int nargs = lua_gettop(L);
    std::stringstream ss;
    
    for (int i = 1; i <= nargs; i++) {
        if (i > 1) {
            ss << "\t";
        }
        
        if (lua_isstring(L, i)) {
            ss << lua_tostring(L, i);
        } else if (lua_isnil(L, i)) {
            ss << "nil";
        } else if (lua_isboolean(L, i)) {
            ss << (lua_toboolean(L, i) ? "true" : "false");
        } else if (lua_isnumber(L, i)) {
            ss << lua_tonumber(L, i);
        } else {
            ss << lua_typename(L, lua_type(L, i));
        }
    }
    
    std::cout << ss.str() << std::endl;
    return 0;
}

// Register built-in functions
bool LuaPlugin::RegisterBuiltins() {
    if (!initialized_ || !luaState_) {
        return false;
    }
    
    // Register print function
    RegisterCFunction("print", LuaPrint);
    
    return true;
}

bool LuaPlugin::RegisterMathFunctions() {
    if (!initialized_ || !luaState_) {
        return false;
    }
    
    // Register Vector3 type and functions
    RegisterVector3(luaState_);
    
    return true;
}

// Template specializations for common types

// Push string to Lua stack
template<>
bool LuaPlugin::PushValue<std::string>(const std::string& value) {
    if (!initialized_ || !luaState_) {
        return false;
    }
    
    lua_pushstring(luaState_, value.c_str());
    return true;
}

// Push number to Lua stack
template<>
bool LuaPlugin::PushValue<double>(const double& value) {
    if (!initialized_ || !luaState_) {
        return false;
    }
    
    lua_pushnumber(luaState_, value);
    return true;
}

// Push integer to Lua stack
template<>
bool LuaPlugin::PushValue<int>(const int& value) {
    if (!initialized_ || !luaState_) {
        return false;
    }
    
    lua_pushinteger(luaState_, value);
    return true;
}

// Push boolean to Lua stack
template<>
bool LuaPlugin::PushValue<bool>(const bool& value) {
    if (!initialized_ || !luaState_) {
        return false;
    }
    
    lua_pushboolean(luaState_, value);
    return true;
}

// Get string from Lua stack
template<>
bool LuaPlugin::GetValue<std::string>(int index, std::string& value) {
    if (!initialized_ || !luaState_) {
        return false;
    }
    
    if (!lua_isstring(luaState_, index)) {
        return false;
    }
    
    value = lua_tostring(luaState_, index);
    return true;
}

// Get number from Lua stack
template<>
bool LuaPlugin::GetValue<double>(int index, double& value) {
    if (!initialized_ || !luaState_) {
        return false;
    }
    
    if (!lua_isnumber(luaState_, index)) {
        return false;
    }
    
    value = lua_tonumber(luaState_, index);
    return true;
}

// Get integer from Lua stack
template<>
bool LuaPlugin::GetValue<int>(int index, int& value) {
    if (!initialized_ || !luaState_) {
        return false;
    }
    
    if (!lua_isinteger(luaState_, index)) {
        return false;
    }
    
    value = static_cast<int>(lua_tointeger(luaState_, index));
    return true;
}

// Get boolean from Lua stack
template<>
bool LuaPlugin::GetValue<bool>(int index, bool& value) {
    if (!initialized_ || !luaState_) {
        return false;
    }
    
    if (!lua_isboolean(luaState_, index)) {
        return false;
    }
    
    value = lua_toboolean(luaState_, index) != 0;
    return true;
}

// Register the plugin
REGISTER_PLUGIN(LuaPlugin)