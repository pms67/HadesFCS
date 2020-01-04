print("Enter minimum, static accelerometer measurements:")
aXmin = float(input("Xmin:"))
aYmin = float(input("Ymin:"))
aZmin = float(input("Zmin:"))

print("Enter maximum, static accelerometer measurements:")
aXmax = float(input("Xmax:"))
aYmax = float(input("Ymax:"))
aZmax = float(input("Zmax:"))

mX = (aXmax - aXmin) / (2 * 9.80655)
mY = (aYmax - aYmin) / (2 * 9.80655)
mZ = (aZmax - aZmin) / (2 * 9.80655)

cX = 9.80655 - mX * aXmax
cY = 9.80655 - mY * aYmax
cZ = 9.80655 - mZ * aZmax

print("Accelerometer scale and bias equations:")
print("aX = " + str(mX) + " * aX' + " + str(cX))
print("aY = " + str(mY) + " * aY' + " + str(cY))
print("aZ = " + str(mZ) + " * aZ' + " + str(cZ))
