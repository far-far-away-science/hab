from enum import Enum

zero = [
    353,
    375,
    396,
    417,
    437,
    457,
    475,
    491,
    506,
    520,
    531,
    541,
    548,
    553,
    555,
    556,
    554,
    550,
    543,
    535,
    524,
    511,
    497,
    480,
    463,
    444,
    424,
    403,
    382,
    360,
    339,
    317,
    296,
    275,
    256,
    237,
    220,
    204,
    190,
    178,
    168,
    160,
    155,
    151,
    150,
    151,
    155,
    160,
    168,
    178,
    190,
    204,
    220,
    237,
    256,
    275,
    296,
    317,
    339
]

one = [
    354,
    423,
    489,
    550,
    604,
    648,
    680,
    700,
    707,
    700,
    680,
    648,
    604,
    550,
    489,
    423,
    354,
    285,
    219,
    158,
    104,
    60,
    28,
    8,
    1,
    8,
    28,
    60,
    104,
    158,
    219,
    285
]

class ItemType(Enum):
    UP = 1
    DOWN = 2
    PEAK = 3
    VALLEY = 4

def getType(array, idx):
    if idx == 0 or idx == len(array) - 1:
        return ItemType.UP # hardcoded
    else:
        previous = array[idx - 1]
        next = array[idx + 1]
        current = array[idx]
        if current < previous and current > next:
            return ItemType.DOWN
        if current > previous and current < next:
            return ItemType.UP
        if current > previous and current > next:
            return ItemType.PEAK
        if current < previous and current < next:
            return ItemType.VALLEY
        assert(False)

def calculateTransition(arrayFrom, arrayTo):
    result = [0] * len(arrayFrom)
    for i in range(len(arrayFrom)):
        index = -1
        distance = float('inf')
        fromType = getType(arrayFrom, i)
        for j in range(len(arrayTo)):
            toType = getType(arrayTo, j)
            if fromType == ItemType.UP and (toType == ItemType.UP or toType == ItemType.PEAK):
                newDistance = abs(arrayTo[j] - arrayFrom[i])
                if arrayTo[j] - arrayFrom[i] >= 0 and newDistance < distance:
                    index = j
                    distance = newDistance
            elif fromType == ItemType.DOWN and (toType == ItemType.DOWN or toType == ItemType.VALLEY):
                newDistance = abs(arrayTo[j] - arrayFrom[i])
                if arrayTo[j] - arrayFrom[i] <= 0 and newDistance < distance:
                    index = j
                    distance = newDistance
            elif fromType == toType:
                newDistance = abs(arrayTo[j] - arrayFrom[i])
                if newDistance < distance:
                    index = j
                    distance = newDistance
        if index == -1:
            # for up have to find closest peak or down
            for j in range(len(arrayTo)):
                toType = getType(arrayTo, j)
                if fromType == ItemType.UP and (toType == ItemType.DOWN or toType == ItemType.PEAK):
                    newDistance = abs(arrayTo[j] - arrayFrom[i])
                    if arrayTo[j] - arrayFrom[i] <= 0 and newDistance < distance:
                        index = j
                        distance = newDistance
                elif fromType == ItemType.DOWN and (toType == ItemType.UP or toType == ItemType.VALLEY):
                    newDistance = abs(arrayTo[j] - arrayFrom[i])
                    if arrayTo[j] - arrayFrom[i] >= 0 and newDistance < distance:
                        index = j
                        distance = newDistance
        if index == -1:
            assert(False)
        result[i] = index
    return result

print('one to zero')
r = calculateTransition(one, zero)
print(len(r))
print(r)
print('zero to one')
r = calculateTransition(zero, one)
print(len(r))
print(r)
