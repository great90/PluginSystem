-- Lua example script for the Plugin System

print("=== Lua Example Script ===")

-- Test Vector3 class
print("\nTesting Vector3 class:")
local v1 = Vector3(1, 2, 3)
local v2 = Vector3(4, 5, 6)
print("v1 = " .. tostring(v1))
print("v2 = " .. tostring(v2))

-- Vector operations
print("\nVector operations:")
local v_add = v1 + v2
local v_sub = v2 - v1
local v_mul = v1 * 2.5
print("v1 + v2 = " .. tostring(v_add))
print("v2 - v1 = " .. tostring(v_sub))
print("v1 * 2.5 = " .. tostring(v_mul))

-- Vector methods
print("\nVector methods:")
local dot_product = v1:dot(v2)
local cross_product = v1:cross(v2)
local length = v1:length()
local normalized = v1:normalize()
print("v1:dot(v2) = " .. dot_product)
print("v1:cross(v2) = " .. tostring(cross_product))
print("v1:length() = " .. length)
print("v1:normalize() = " .. tostring(normalized))

-- Test math functions
print("\nTesting math functions:")

-- Create a rotation quaternion
local angle_deg = 45.0
local angle_rad = math.rad(angle_deg)
local axis = Vector3(0, 1, 0)

-- Test rotation if available
if v1.rotated then
    local rotated = v1:rotated(axis, angle_rad)
    print("v1 rotated " .. angle_deg .. " degrees around Y-axis = " .. tostring(rotated))
else
    print("Rotation method not available")
end

-- Test lerp function if available
if math.lerp then
    local t = 0.5
    local lerped = math.lerp(v1, v2, t)
    print("math.lerp(v1, v2, " .. t .. ") = " .. tostring(lerped))
else
    print("Lerp function not available")
end

-- Test random function
math.randomseed(os.time())
local rand = math.random(1, 10)
print("math.random(1, 10) = " .. rand)

-- Define a function that can be called from C++
function calculate_fibonacci(n)
    if n <= 0 then
        return 0
    elseif n == 1 then
        return 1
    else
        local a, b = 0, 1
        for i = 2, n do
            a, b = b, a + b
        end
        return b
    end
end

-- Test the function
print("\nTesting Fibonacci function:")
for i = 0, 9 do
    print("Fibonacci(" .. i .. ") = " .. calculate_fibonacci(i))
end

print("\n=== Lua Example Script Completed ===")

-- Return a value that can be retrieved by the host application
return "Lua script executed successfully"