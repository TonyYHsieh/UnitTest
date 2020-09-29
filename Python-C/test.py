import fputs

print(fputs.__doc__)
print(fputs.__name__)

# Write to an empty file named `write.txt`
ret = fputs.fputs("Real Python!", "write.txt")
print("fputs call return {}".format(ret))

with open("write.txt", "r") as f:
    print(f.read())

print('fputs.FPUTS_FLAG = {}'.format(fputs.FPUTS_FLAG))
print('fputs.FPUTS_MACRO = {}'.format(fputs.FPUTS_MACRO))
