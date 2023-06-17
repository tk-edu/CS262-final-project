import base64
import sys
mapp = []
def decode(flag,r):
  match r:
    case '0':
      with open(flag, "rb") as file:
        decoded_string = base64.b64decode(file.read())
      # print(decoded_string)
      with open(flag,'wb') as file:
        file.write(decoded_string)
    case '1':
      with open(flag, "rb") as file:
        decoded_string = base64.b32decode(file.read())
      # print(decoded_string)
      with open(flag,'wb') as file:
        file.write(decoded_string)
    case '2':
      with open(flag, "rb") as file:
        decoded_string = base64.b16decode(file.read())
      # print(decoded_string)
      with open(flag,'wb') as file:
        file.write(decoded_string)
    case '3':
      with open(flag, "rb") as file:
        decoded_string = base64.b85decode(file.read())
      # print(decoded_string)
      with open(flag,'wb') as file:
        file.write(decoded_string)
# mapfile = open('map.txt','r')
# mapstr = mapfile.read()
# mapfile.close()

# for r in range(len(mapstr)):
#   decode('flag.txt', mapstr[r])

if __name__ == '__main__':
  # file name, encoding type
  decode(sys.argv[1], sys.argv[2])