#encoding file
import base64
import random
import sys
mapp = []
  
#can add more encoding methods under casses just have these two for now
def encode(flag,r):
  match r:
    case '0':
      with open(flag, "rb") as file:
        encoded_string = base64.b64encode(file.read())
      with open(flag,'wb') as file:
        file.write(encoded_string)
    case '1':
      with open(flag, "rb") as file:
        encoded_string = base64.b32encode(file.read())
      with open(flag,'wb') as file:
        file.write(encoded_string)
    case '2':
      with open(flag, "rb") as file:
        encoded_string = base64.b16encode(file.read())
      with open(flag,'wb') as file:
        file.write(encoded_string)
    case '3':
      with open(flag, "rb") as file:
        encoded_string = base64.b85encode(file.read())
      with open(flag,'wb') as file:
        file.write(encoded_string)
      
## round is how many times you encode the flag
# for round in range(30):
#   r=str(random.randint(0,3))
#   mapp.append(r)
#   encode('flag.txt',r)
  
# mappp =[str(x) for x in mapp.__reversed__()]
# strmappp= ''.join(map(str,mappp))
# print(type(strmappp))
# mapfile = open('map.txt','wb')
# mapfile.write(strmappp.encode('ascii'))
# mapfile.close()

if __name__ == '__main__':
  # file name, encoding type
  encode(sys.argv[1], sys.argv[2])