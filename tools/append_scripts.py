# Append script files to exe
with open('bin\\mainXOR.exe', 'ab') as exe:
    with open('tools\\encode.py', 'rb') as encode_script:
        for line in encode_script:
            exe.write(line)
    with open('tools\\decode.py', 'rb') as decode_script:
        for line in decode_script:
            exe.write(line)