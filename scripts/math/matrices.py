import sympy as sym

# symbolic stuff for projection matrices.
r,t,n = sym.symbols('r t n')
m1 = sym.Matrix([[r, t, n, 1],
                 [-r, t, n, 1],
                 [-r, -t, n, 1],
                 [0,0,0,1]])
m2 = sym.Matrix([[1,1,0,1],
                 [-1,1,0,1],
                 [-1,-1,0,1],
                 [0,0,-1,0]])

print("m1: ", m1)
print("m2: ", m2)
print("m1^-1: ", m1.inv())
print("m1^-1 * m2: ", m1.inv() * m2)


