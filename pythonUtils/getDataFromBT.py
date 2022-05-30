import serial #Library: pyserial

def checkHeader(dataHeader, lineString):
    if dataHeader in lineString:
        return True
    return False

def getDataBT(sp, dataHeader = "ATT", lineEnd = "\n", expectedSize = 3):
    EOL = False
    lineString = ''
    try:
        while not EOL:
            data = sp.read(1)
            if data:
                char = data.decode('ascii')
                EOL = (char == lineEnd)
                if not EOL:
                    lineString += char
            else:
                lineString = None

        if checkHeader(dataHeader, lineString):
            lineSplit = lineString.split(' ')
            dataArray = [float(x) for x in lineSplit[1:]]
            if len(dataArray) != expectedSize:
                return None
            print(dataArray)
            return dataArray
        return None

    except Exception as e:
        print(e)
        return None