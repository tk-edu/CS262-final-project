# Append zip file to exe
with open('bin\\mainXOR.exe', 'ab') as exe:
    with open('gameShowSite\\site.zip', 'rb') as zip:
        for line in zip:
            exe.write(line)