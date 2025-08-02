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

// C function to be called from Lua
static int LuaPrint(lua_State* L) {
    int nargs = lua_gettop(L);
    std::string message;
    
    for (int i = 1; i <= nargs; i++) {
        if (lua_isstring(L, i)) {
            message += lua_tostring(L, i);
        }
        else if (lua_isnumber(L, i)) {
            message += std::to_string(lua_tonumber(L, i));
        }
        else if (lua_isboolean(L, i)) {
            message += lua_toboolean(L, i) ? "true" : "false";
        }
        else if (lua_isnil(L, i)) {
            message += "nil";
        }
        else {
            message += lua_typename(L, lua_type(L, i));
        }
        
        if (i < nargs) {
            message += "\t";
        }
    }
    
    std::cout << "[Lua] " << message << std::endl;
    return 0;
}

// Vector3 wrapper for Lua
static const char* VECTOR3_METATABLE = "Vector3";

// Constructor for Vector3 in Lua
static int Vector3_New(lua_State* L) {
    float x = static_cast<float>(luaL_optnumber(L, 1, 0.0));
    float y = static_cast<float>(luaL_optnumber(L, 2, 0.0));
    float z = static_cast<float>(luaL_optnumber(L, 3, 0.0));
    
    // Create a new Vector3 userdata
    Vector3* vec = static_cast<Vector3*>(lua_newuserdata(L, sizeof(Vector3)));
    new(vec) Vector3(x, y, z);
    
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
    
    Vector3* result = static_cast<Vector3*>(lua_newuserdata(L, sizeof(Vector3)));
    new(result) Vector3(a->x + b->x, a->y + b->y, a->z + b->z);
    
    luaL_getmetatable(L, VECTOR3_METATABLE);
    lua_setmetatable(L, -2);
    
    return 1;
}

// Vector3 subtraction
static int Vector3_Sub(lua_State* L) {
    Vector3* a = CheckVector3(L, 1);
    Vector3* b = CheckVector3(L, 2);
    
    Vector3* result = static_cast<Vector3*>(lua_newuserdata(L, sizeof(Vector3)));
    new(result) Vector3(a->x - b->x, a->y - b->y, a->z - b->z);
    
    luaL_getmetatable(L, VECTOR3_METATABLE);
    lua_setmetatable(L, -2);
    
    return 1;
}

// Vector3 dot product
static int Vector3_Dot(lua_State* L) {
    Vector3* a = CheckVector3(L, 1);
    Vector3* b = CheckVector3(L, 2);
    
    float dot = a->x * b->x + a->y * b->y + a->z * b->z;
    lua_pushnumber(L, dot);
    
    return 1;
}

// Vector3 cross product
static int Vector3_Cross(lua_State* L) {
    Vector3* a = CheckVector3(L, 1);
    Vector3* b = CheckVector3(L, 2);
    
    Vector3* result = static_cast<Vector3*>(lua_newuserdata(L, sizeof(Vector3)));
    new(result) Vector3(
        a->y * b->z - a->z * b->y,
        a->z * b->x - a->x * b->z,
        a->x * b->y - a->y * b->x
    );
    
    luaL_getmetatable(L, VECTOR3_METATABLE);
    lua_setmetatable(L, -2);
    
    return 1;
}

// Vector3 length
static int Vector3_Length(lua_State* L) {
    Vector3* vec = CheckVector3(L, 1);
    float length = std::sqrt(vec->x * vec->x + vec->y * vec->y + vec->z * vec->z);
    lua_pushnumber(L, length);
    return 1;
}

// Vector3 normalize
static int Vector3_Normalize(lua_State* L) {
    Vector3* vec = CheckVector3(L, 1);
    float length = std::sqrt(vec->x * vec->x + vec->y * vec->y + vec->z * vec->z);
    
    if (length > 0.0001f) {
        Vector3* result = static_cast<Vector3*>(lua_newuserdata(L, sizeof(Vector3)));
        new(result) Vector3(vec->x / length, vec->y / length, vec->z / length);
        
        luaL_getmetatable(L, VECTOR3_METATABLE);
        lua_setmetatable(L, -2);
    } else {
        // Return a copy of the zero vector
        Vector3* result = static_cast<Vector3*>(lua_newuserdata(L, sizeof(Vector3)));
        new(result) Vector3(*vec);
        
        luaL_getmetatable(L, VECTOR3_METATABLE);
        lua_setmetatable(L, -2);
    }
    
    return 1;
}

// Vector3 tostring
static int Vector3_ToString(lua_State* L) {
    Vector3* vec = CheckVector3(L, 1);
    std::stringstream ss;
    ss << "Vector3(" << vec->x << ", " << vec->y << ", " << vec->z << ")";
    lua_pushstring(L, ss.str().c_str());
    return 1;
}

// Vector3 index (get component)
static int Vector3_Index(lua_State* L) {
    Vector3* vec = CheckVector3(L, 1);
    const char* key = luaL_checkstring(L, 2);
    
    if (strcmp(key, "x") == 0) {
        lua_pushnumber(L, vec->x);
    } else if (strcmp(key, "y") == 0) {
        lua_pushnumber(L, vec->y);
    } else if (strcmp(key, "z") == 0) {
        lua_pushnumber(L, vec->z);
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
    
    if (strcmp(key, "x") == 0) {
        vec->x = value;
    } else if (strcmp(key, "y") == 0) {
        vec->y = value;
    } else if (strcmp(key, "z") == 0) {
        vec->z = value;
    } else {
        luaL_error(L, "Cannot set invalid Vector3 component: %s", key);
    }
    
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

// LuaPlugin implementation

LuaPlugin::LuaPlugin()
    : luaState_(nullptr), initialized_(false) {
}

LuaPlugin::~LuaPlugin() {
    Shutdown();
}

bool LuaPlugin::Initialize() {
    if (initialized_) {
        return true;
    }
    
    if (!InitializeLua()) {
        std::cerr << "Failed to initialize Lua interpreter" << std::endl;
        return false;
    }
    
    if (!RegisterBuiltins()) {
        std::cerr << "Failed to register built-in functions" << std::endl;
        FinalizeLua();
        return false;
    }
    
    if (!RegisterMathFunctions()) {
        std::cerr << "Failed to register math functions" << std::endl;
        FinalizeLua();
        return false;
    }
    
    initialized_ = true;
    std::cout << "LuaPlugin initialized successfully" << std::endl;
    return true;
}

void LuaPlugin::Shutdown() {
    if (initialized_) {
        FinalizeLua();
        initialized_ = false;
        std::cout << "LuaPlugin shut down" << std::endl;
    }
}

const PluginInfo& LuaPlugin::GetPluginInfo() const {
    return pluginInfo_;
}

std::string LuaPlugin::Serialize() {
    // For now, we don't have any state to serialize
    return "";
}

bool LuaPlugin::Deserialize(const std::string& data) {
    // For now, we don't have any state to deserialize
    return true;
}

bool LuaPlugin::PrepareForHotReload() {
    // Nothing special needed for hot reload preparation
    return true;
}

bool LuaPlugin::CompleteHotReload() {
    // Re-register all functions and objects after hot reload
    if (initialized_) {
        if (!RegisterBuiltins() || !RegisterMathFunctions()) {
            return false;
        }
    }
    return true;
}

bool LuaPlugin::ExecuteFile(const std::string& filePath) {
    if (!initialized_ || !luaState_) {
        std::cerr << "Lua interpreter not initialized" << std::endl;
        return false;
    }
    
    if (!FileExists(filePath)) {
        std::cerr << "File not found: " << filePath << std::endl;
        return false;
    }
    
    int result = luaL_dofile(luaState_, filePath.c_str());
    return !HandleLuaError(result);
}

bool LuaPlugin::ExecuteString(const std::string& script) {
    if (!initialized_ || !luaState_) {
        std::cerr << "Lua interpreter not initialized" << std::endl;
        return false;
    }
    
    int result = luaL_dostring(luaState_, script.c_str());
    return !HandleLuaError(result);
}

bool LuaPlugin::EvaluateExpression(const std::string& expression, std::string& result) {
    if (!initialized_ || !luaState_) {
        std::cerr << "Lua interpreter not initialized" << std::endl;
        return false;
    }
    
    // Wrap the expression to return its value
    std::string script = "return " + expression;
    
    int luaResult = luaL_dostring(luaState_, script.c_str());
    if (luaResult != LUA_OK) {
        // Try without the return statement
        luaResult = luaL_dostring(luaState_, expression.c_str());
        if (HandleLuaError(luaResult)) {
            return false;
        }
        result = "";
        return true;
    }
    
    // Get the result from the stack
    if (lua_isstring(luaState_, -1)) {
        result = lua_tostring(luaState_, -1);
    } else if (lua_isnumber(luaState_, -1)) {
        result = std::to_string(lua_tonumber(luaState_, -1));
    } else if (lua_isboolean(luaState_, -1)) {
        result = lua_toboolean(luaState_, -1) ? "true" : "false";
    } else if (lua_isnil(luaState_, -1)) {
        result = "nil";
    } else {
        // For other types, just return the type name
        result = lua_typename(luaState_, lua_type(luaState_, -1));
    }
    
    // Pop the result from the stack
    lua_pop(luaState_, 1);
    
    return true;
}

bool LuaPlugin::RegisterFunction(const std::string& name, void* function) {
    if (!initialized_ || !luaState_) {
        std::cerr << "Lua interpreter not initialized" << std::endl;
        return false;
    }
    
    // We need to cast the void* to a lua_CFunction
    lua_CFunction luaFunc = reinterpret_cast<lua_CFunction>(function);
    return RegisterCFunction(name, luaFunc);
}

bool LuaPlugin::RegisterObject(const std::string& name, void* object) {
    if (!initialized_ || !luaState_) {
        std::cerr << "Lua interpreter not initialized" << std::endl;
        return false;
    }
    
    // For now, we don't have a generic way to register arbitrary objects
    // This would require more complex binding code
    std::cerr << "RegisterObject not fully implemented for LuaPlugin" << std::endl;
    return false;
}

std::vector<std::string> LuaPlugin::GetSupportedExtensions() const {
    return {".lua"};
}

std::string LuaPlugin::GetLanguageName() const {
    return "Lua";
}

std::string LuaPlugin::GetLanguageVersion() const {
    if (initialized_ && luaState_) {
        // Get the Lua version from the global _VERSION variable
        lua_getglobal(luaState_, "_VERSION");
        if (lua_isstring(luaState_, -1)) {
            std::string version = lua_tostring(luaState_, -1);
            lua_pop(luaState_, 1);
            return version;
        }
        lua_pop(luaState_, 1);
    }
    
    // Fallback to a default version
    return "Lua 5.4";
}

lua_State* LuaPlugin::GetLuaState() const {
    return luaState_;
}

bool LuaPlugin::RegisterCFunction(const std::string& name, lua_CFunction function) {
    if (!initialized_ || !luaState_) {
        return false;
    }
    
    lua_pushcfunction(luaState_, function);
    lua_setglobal(luaState_, name.c_str());
    
    return true;
}

bool LuaPlugin::CallFunction(const std::string& functionName, int numArgs, int numResults) {
    if (!initialized_ || !luaState_) {
        return false;
    }
    
    // Get the function from the global table
    lua_getglobal(luaState_, functionName.c_str());
    
    // Check if it's a function
    if (!lua_isfunction(luaState_, -1)) {
        std::cerr << "Function not found: " << functionName << std::endl;
        lua_pop(luaState_, 1);  // Pop the non-function value
        return false;
    }
    
    // Move the function before the arguments
    if (numArgs > 0) {
        lua_insert(luaState_, -(numArgs + 1));
    }
    
    // Call the function
    int result = lua_pcall(luaState_, numArgs, numResults, 0);
    return !HandleLuaError(result);
}

bool LuaPlugin::InitializeLua() {
    // Create a new Lua state
    luaState_ = luaL_newstate();
    if (!luaState_) {
        std::cerr << "Failed to create Lua state" << std::endl;
        return false;
    }
    
    // Open standard libraries
    luaL_openlibs(luaState_);
    
    return true;
}

void LuaPlugin::FinalizeLua() {
    if (luaState_) {
        lua_close(luaState_);
        luaState_ = nullptr;
    }
}

bool LuaPlugin::HandleLuaError(int result) {
    if (result != LUA_OK) {
        std::string errorMsg = "Lua error: ";
        if (lua_isstring(luaState_, -1)) {
            errorMsg += lua_tostring(luaState_, -1);
        } else {
            errorMsg += "Unknown error";
        }
        
        std::cerr << errorMsg << std::endl;
        lua_pop(luaState_, 1);  // Pop the error message
        return true;  // Error occurred
    }
    
    return false;  // No error
}

bool LuaPlugin::RegisterBuiltins() {
    if (!initialized_ || !luaState_) {
        return false;
    }
    
    // Register custom print function
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