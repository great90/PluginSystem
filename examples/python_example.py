# Python example script for the Plugin System

print("=== Python Example Script ===")

# Import the math module if available
try:
    import math
    print("Math module imported successfully")
except ImportError:
    print("Failed to import math module")

# Test Vector3 class
print("\nTesting Vector3 class:")
v1 = Vector3(1, 2, 3)
v2 = Vector3(4, 5, 6)
print(f"v1 = {v1}")
print(f"v2 = {v2}")

# Vector operations
print("\nVector operations:")
v_add = v1 + v2
v_sub = v2 - v1
v_mul = v1 * 2.5
print(f"v1 + v2 = {v_add}")
print(f"v2 - v1 = {v_sub}")
print(f"v1 * 2.5 = {v_mul}")

# Vector methods
print("\nVector methods:")
dot_product = v1.dot(v2)
cross_product = v1.cross(v2)
length = v1.length()
normalized = v1.normalize()
print(f"v1.dot(v2) = {dot_product}")
print(f"v1.cross(v2) = {cross_product}")
print(f"v1.length() = {length}")
print(f"v1.normalize() = {normalized}")

# Test math functions if available
try:
    print("\nTesting math functions:")
    # Create a rotation quaternion
    angle_deg = 45.0
    angle_rad = math.radians(angle_deg)
    axis = Vector3(0, 1, 0)
    
    # Rotate a vector
    rotated = v1.rotated(axis, angle_rad)
    print(f"v1 rotated {angle_deg} degrees around Y-axis = {rotated}")
    
    # Test lerp function
    t = 0.5
    lerped = math.lerp(v1, v2, t)
    print(f"math.lerp(v1, v2, {t}) = {lerped}")
    
    # Test random function
    rand = math.random(1, 10)
    print(f"math.random(1, 10) = {rand}")
except Exception as e:
    print(f"Error testing math functions: {e}")

# Define a function that can be called from C++
def calculate_fibonacci(n):
    """Calculate the nth Fibonacci number"""
    if n <= 0:
        return 0
    elif n == 1:
        return 1
    else:
        a, b = 0, 1
        for _ in range(2, n + 1):
            a, b = b, a + b
        return b

# Test the function
print("\nTesting Fibonacci function:")
for i in range(10):
    print(f"Fibonacci({i}) = {calculate_fibonacci(i)}")

print("\n=== Python Example Script Completed ===")