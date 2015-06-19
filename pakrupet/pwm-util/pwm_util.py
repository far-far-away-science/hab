from enum import Enum

f1200 = [
    354,
    386,
    418,
    450,
    480,
    509,
    536,
    560,
    583,
    603,
    620,
    634,
    645,
    652,
    656,
    657,
    654,
    648,
    638,
    625,
    609,
    590,
    568,
    544,
    518,
    490,
    460,
    429,
    397,
    365,
    332,
    300,
    269,
    238,
    209,
    181,
    156,
    132,
    111,
    93,
    78,
    67,
    58,
    53,
    51,
    53,
    58,
    67,
    78,
    93,
    111,
    132,
    156,
    181,
    209,
    238,
    269,
    300,
    332
]

f2200 = [
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

print('2200Hz')
print(f2200)
print('2200Hz to 1200Hz')
r = calculateTransition(f2200, f1200)
print(len(r))
print(r)
print('1200Hz')
print(f1200)
print('1200Hz to 2200Hz')
r = calculateTransition(f1200, f2200)
print(len(r))
print(r)
