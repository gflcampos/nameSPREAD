import json
import sys
import os

usageMsg = 'Select a scenario: 2, 3, 4 or 5.' 

def exists(pair, pairsList):
    for i in range(0, len(pairsList)):
        if pair[0] in pairsList[i] and pair[1] in pairsList[i]:
            return True
    return False

def getJsonData(caseNum):
    data = []
    pathToJsons = 'scenarios/%s/' % caseNum
    jsonFiles = [file for file in os.listdir(pathToJsons) if file.endswith('.json')]

    for i in range(0, len(jsonFiles)):
        with open('%s%s' % (pathToJsons, jsonFiles[i])) as data_file:
            data.append(json.load(data_file))

    return data

def getUniqueLinks(data):
    links = []

    for i in range(0, len(data)):
        allLinks = data[i]["collection"][0]["links"]

        for j in range(0, len(allLinks)):
            src = allLinks[j]["source"]
            dst = allLinks[j]["target"]
            link = [src, dst]
            if not(exists(link, links)):
                links.append(link)
    return links

def main():
    if (len(sys.argv) < 2):
        print(usageMsg)
        sys.exit(0)

    caseNum = int(sys.argv[1])
    links = []

    if (caseNum == 2):
        nodes = 10
    elif (caseNum == 3):
        nodes = 30
    elif (caseNum == 4 or caseNum == 5):
        nodes = 100
    else:
        print(usageMsg)
        sys.exit(0)

    data = getJsonData(caseNum)
    links = getUniqueLinks(data)

    """ calculate density """
    actualLinks = len(links)
    potentialLinks = (nodes * (nodes - 1)) / 2
    density = actualLinks/float(potentialLinks)

    print "Potential links: " + str(potentialLinks)
    print "Actual links: " + str(actualLinks)
    print "Network density: " + str(density)

main()
