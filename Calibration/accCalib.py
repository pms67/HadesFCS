print("Enter minimum, static accelerometer measurements:")
aXmin = float(input("Xmin:"))
aYmin = float(input("Ymin:"))
aZmin = float(input("Zmin:"))

print("Enter maximum, static accelerometer measurements:")
aXmax = float(input("Xmax:"))
aYmax = float(input("Ymax:"))
aZmax = float(input("Zmax:"))

mX = (aXmax - aXmin) / 19.62
mY = (aYmax - aYmin) / 19.62
mZ = (aZmax - aZmin) / 19.62

cX = 9.81 - mX * aXmax
cY = 9.81 - mY * aYmax
cZ = 9.81 - mZ * aZmax

print("Accelerometer scale and bias equations:")
print("aX = " + str(mX) + " * aX' + " + str(cX))
print("aY = " + str(mY) + " * aY' + " + str(cY))
print("aZ = " + str(mZ) + " * aZ' + " + str(cZ))
