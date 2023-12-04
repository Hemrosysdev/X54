# -*- coding: utf-8 -*-
#
# Configuration file for the Sphinx documentation builder.
#
# This file does only contain a selection of the most common options. For a
# full list see the documentation:
# http://www.sphinx-doc.org/en/stable/config

from sphinx_multibuild import SphinxMultiBuilder
import logging
import time
import getopt
import sys
import os
import string
import re

def replaceData(character, counter):
    TextLine = ""
    
    for i in range(counter):
        TextLine = TextLine + character
    return TextLine

# Set the source-path
if ((len(sys.argv) != 3) and (sys.argv[1] != '-C')):
    exit(1)
SourcePath = sys.argv[2]

# Package respects loglevel set by application. Info prints out change events
# in input directories and warning prints exception that occur during symlink
# creation/deletion.
loglevel = logging.INFO
logging.basicConfig(format='%(message)s', level=loglevel)

# --------------------------------------------------------------------
# check the list-of-testcase-chapters.rst file
# --------------------------------------------------------------------

if os.path.isfile(SourcePath + "/source/list-of-testcase-chapters.rst"):
    f1 = open(SourcePath + "/source/list-of-testcase-chapters.rst", "r")

    Filedata = f1.read()
    FileDataLines = Filedata.split("\n")

    f1.close()

    DelTestcase = False
    i = 0
    CounterStack = []
    ChapterNr = -1
    ChapterNrOld = -1
    ChapterCounter = [0, 0, 0, 0, 0, 0, 0, 0]
    TcGroupCounter = [0, 0, 0, 0, 0, 0, 0, 0]
    TitleStack = ["", "", "", "", "", "", "", ""]
    TcNr = 1
    LastChapterValid = [False, False, False, False, False, False, False, False]
    LenStart = False
    toctreeStart = False
    ChapterActiv = False
    MaxLen = 0
    length = 0
    CurChapterText = ""
    
    while (i < len(FileDataLines)):
        if (FileDataLines[i].find("======") != -1):
            TcNr = 1
            TcGroupCounter[0] = TcGroupCounter[0] + 1
            TitleStack[0] = FileDataLines[i-1].strip()
            
            for k in range(len(TcGroupCounter) - 1):
                TcGroupCounter[k + 1] = 0
                TitleStack[k + 1] = ""
            
            FileDataLines[i] = replaceData("=", len(TitleStack[0]))
            CounterStack.append(i - 3)
            ChapterNrOld = ChapterNr
            ChapterNr = 0
            LenStart = True
            SetToctree = True
            ChapterActiv = True
            CurChapterText = "Chapter: " + TitleStack[0]
          
        elif (FileDataLines[i].find("------") != -1):
            TcNr = 1
            TcGroupCounter[1] = TcGroupCounter[1] + 1
            TitleStack[1] = FileDataLines[i-1].strip()
            
            for k in range(len(TcGroupCounter) - 2):
                TcGroupCounter[k + 2] = 0
                TitleStack[k + 2] = ""
            
            FileDataLines[i] = replaceData("-", len(TitleStack[1]))
            CounterStack.append(i - 3)
            ChapterNrOld = ChapterNr
            ChapterNr = 1
            LenStart = True
            SetToctree = True
            ChapterActiv = True
            CurChapterText = "Chapter: " + TitleStack[1]
            
        elif (FileDataLines[i].find("``````") != -1):
            TcNr = 1
            TcGroupCounter[2] = TcGroupCounter[2] + 1
            TitleStack[2] = FileDataLines[i-1].strip()
            
            for k in range(len(TcGroupCounter) - 3):
                TcGroupCounter[k + 3] = 0
                TitleStack[k + 3] = ""
            
            FileDataLines[i] = replaceData("`", len(TitleStack[2]))
            CounterStack.append(i - 3)
            ChapterNrOld = ChapterNr
            ChapterNr = 2
            LenStart = True
            SetToctree = True
            ChapterActiv = True
            CurChapterText = "Chapter: " + TitleStack[2]
            
        elif (FileDataLines[i].find("......") != -1):
            TcNr = 1
            TcGroupCounter[3] = TcGroupCounter[3] + 1
            TitleStack[3] = FileDataLines[i-1].strip()
            
            for k in range(len(TcGroupCounter) - 4):
                TcGroupCounter[k + 4] = 0
                TitleStack[k + 4] = ""
            
            Title = TitleStack[2] + " -> " + TitleStack[3]
            FileDataLines[i - 1] = Title
            FileDataLines[i] = replaceData("`", len(Title))
            CounterStack.append(i - 3)
            ChapterNrOld = ChapterNr
            ChapterNr = 3
            LenStart = True
            SetToctree = True
            ChapterActiv = True
            CurChapterText = "Chapter: " + Title
            
        elif (FileDataLines[i].find("~~~~~~") != -1):
            TcNr = 1
            TcGroupCounter[4] = TcGroupCounter[4] + 1
            TitleStack[4] = FileDataLines[i-1].strip()
            
            for k in range(len(TcGroupCounter) - 5):
                TcGroupCounter[k + 5] = 0
                TitleStack[k + 5] = ""
                        
            Title = TitleStack[2] + " -> " + TitleStack[3] + " -> " + TitleStack[4]
            FileDataLines[i - 1] = Title
            FileDataLines[i] = replaceData("`", len(Title))
            CounterStack.append(i - 3)
            ChapterNrOld = ChapterNr
            ChapterNr = 4
            LenStart = True
            SetToctree = True
            ChapterActiv = True
            CurChapterText = "Chapter: " + Title
            
        elif (FileDataLines[i].find("******") != -1):
            TcNr = 1
            TcGroupCounter[5] = TcGroupCounter[5] + 1
            TitleStack[5] = FileDataLines[i-1].strip()
            
            for k in range(len(TcGroupCounter) - 6):
                TcGroupCounter[k + 6] = 0
                TitleStack[k + 6] = ""
                        
            Title = TitleStack[2] + " -> " + TitleStack[3] + " -> " + TitleStack[4] + " -> " + TitleStack[5]
            FileDataLines[i - 1] = Title
            FileDataLines[i] = replaceData("`", len(Title))
            CounterStack.append(i - 3)
            ChapterNrOld = ChapterNr
            ChapterNr = 5
            LenStart = True
            SetToctree = True
            ChapterActiv = True
            CurChapterText = "Chapter: " + Title
            
        elif (FileDataLines[i].find("++++++") != -1):
            TcNr = 1
            TcGroupCounter[6] = TcGroupCounter[6] + 1
            TitleStack[6] = FileDataLines[i-1].strip()
            
            for k in range(len(TcGroupCounter) - 7):
                TcGroupCounter[k + 7] = 0
                TitleStack[k + 7] = ""
                        
            Title = TitleStack[2] + " -> " + TitleStack[3] + " -> " + TitleStack[4] + " -> " + TitleStack[5] + " -> " + TitleStack[6]
            FileDataLines[i - 1] = Title
            FileDataLines[i] = replaceData("`", len(Title))
            CounterStack.append(i - 3)
            ChapterNrOld = ChapterNr
            ChapterNr = 6
            LenStart = True
            SetToctree = True
            ChapterActiv = True
            CurChapterText = "Chapter: " + Title
            
        elif (FileDataLines[i].find("^^^^^^") != -1):
            TcNr = 1
            TcGroupCounter[7] = TcGroupCounter[7] + 1
            TitleStack[7] = FileDataLines[i-1].strip()
                        
            Title = TitleStack[2] + " -> " + TitleStack[3] + " -> " + TitleStack[4] + " -> " + TitleStack[5] + " -> " + TitleStack[6] + " -> " + TitleStack[7]
            FileDataLines[i - 1] = Title
            FileDataLines[i] = replaceData("`", len(Title))
            CounterStack.append(i - 3)
            ChapterNrOld = ChapterNr
            ChapterNr = 7
            LenStart = True
            SetToctree = True
            ChapterActiv = True
            CurChapterText = "Chapter: " + Title
        
        elif ((FileDataLines[i].find(".. _") != -1) and (ChapterNr != -1)):
            if (ChapterNr == ChapterNrOld):
                if LastChapterValid[ChapterNr]:
                    if ((MaxLen == 0) and SetToctree):
                        LastChapterValid[ChapterNr] = False
                    else:
                        ChapterCounter[ChapterNr] = ChapterCounter[ChapterNr] + 1

                else:
                    if ((MaxLen > 0) or (SetToctree == False)):
                        LastChapterValid[ChapterNr] = True
                        ChapterCounter[ChapterNr] = ChapterCounter[ChapterNr] + 1

                    LastPos1 = CounterStack.pop()
                    LastPos2 = CounterStack.pop()

                    for counter in range(LastPos1 - LastPos2):
                        del FileDataLines[LastPos2]
                        i = i - 1

                    CounterStack.append(LastPos2)

            elif (ChapterNr < ChapterNrOld):
                counter = ChapterNrOld
                DeleteLast = True
                CurrentPos = CounterStack.pop()
                
                while (counter > ChapterNr):
                    if (ChapterCounter[counter] > 0):
                        DeleteLast = False
                        
                        if ((counter == ChapterNrOld) and not LastChapterValid[ChapterNrOld]):
                            LastPos = CounterStack.pop()

                            for counter2 in range(CurrentPos - LastPos):
                                del FileDataLines[LastPos]
                                i = i - 1
                            CurrentPos = LastPos
                    
                    elif DeleteLast:
                        LastPos = CounterStack.pop()

                        for counter2 in range(CurrentPos - LastPos):
                            del FileDataLines[LastPos]
                            i = i - 1
                        CurrentPos = LastPos

                    LastChapterValid[counter] = False
                    ChapterCounter[counter] = 0
                    counter = counter - 1

                if not DeleteLast:
                    if ((MaxLen == 0) and SetToctree):
                        LastChapterValid[ChapterNr] = False
                    else:
                        ChapterCounter[ChapterNr] = ChapterCounter[ChapterNr] + 1
                        LastChapterValid[ChapterNr] = True

                    CounterStack.append(CurrentPos)

                elif LastChapterValid[ChapterNr]:
                    if ((MaxLen == 0) and SetToctree):
                        LastChapterValid[ChapterNr] = False
                    else:
                        ChapterCounter[ChapterNr] = ChapterCounter[ChapterNr] + 1

                    CounterStack.append(CurrentPos)

                else:
                    if ((MaxLen > 0) or (SetToctree == False)):
                        LastChapterValid[ChapterNr] = True
                        ChapterCounter[ChapterNr] = ChapterCounter[ChapterNr] + 1

                    LastPos2 = CounterStack.pop()

                    for counter in range(CurrentPos - LastPos2):
                        del FileDataLines[LastPos2]
                        i = i - 1

                    CounterStack.append(LastPos2)

            else:
                if ((MaxLen > 0) or (SetToctree == False)):
                    LastChapterValid[ChapterNr] = True
                    ChapterCounter[ChapterNr] = ChapterCounter[ChapterNr] + 1
                else:
                    LastChapterValid[ChapterNr] = False

            LenStart = False
            ChapterActiv = False
            MaxLen = 0
            print("{} --> {}".format(CurChapterText, LastChapterValid[ChapterNr]))
            
        elif ((FileDataLines[i].find("<tc") != -1) and (FileDataLines[i].find(">`") != -1)):
            Pos1 = FileDataLines[i].find("<tc")
            Pos2 = FileDataLines[i].find(">`")
            TestcaseFile = SourcePath + "/" + FileDataLines[i][Pos1 + 1:Pos2].strip() + ".rst"
            DelTestcase = False

            if os.path.isfile(TestcaseFile):
                f2 = open(TestcaseFile, "r")

                Filedata = f2.read()
                TestcaseLines = Filedata.split("\n")
                j = 0
                ReadStatus = 0

                f2.close()
        
                while (j < len(TestcaseLines)):
                    if (ReadStatus == 0):
                        if TestcaseLines[j].find("=====") != -1:
                            Text = TestcaseLines[j-1].replace("_", " ")

                            for k in range(len(TitleStack)):
                                if len(TitleStack[k]) > 0:
                                    Text = Text.replace(TitleStack[k] + " ", "")
                        
                            #if len(Text) > 85:
                            #    Text = Text[:85] + " ..."
                            
                            TestcaseLines[j-1] = Text
                            TestcaseLines[j] = replaceData("=", len(TestcaseLines[j-1]))

                        elif TestcaseLines[j].find("**Assemplies**") != -1:
                            TestcaseLines.insert(j + 1, "")
                            TestcaseLines.insert(j, "")
                            j = j + 2
                            ReadStatus = 1

                        #else:
                        #    TestcaseLines[j] = TestcaseLines[j].replace("_", " ")

                    elif (ReadStatus == 1):
                        if TestcaseLines[j].find("**Test description**") != -1:
                            j = j + 1
                            ReadStatus = 2

                        elif len(TestcaseLines[j].strip()) == 0:
                            del TestcaseLines[j]
                            j = j - 1

                        else:
                            TestcaseLines[j] = "- " + TestcaseLines[j].replace(",", "")

                    else:
                        if TestcaseLines[j].find("--------") != -1:
                            TestcaseLines[j - 2] = "*" + TestcaseLines[j - 2].strip() + "*"
                            del TestcaseLines[j]
                            j = j - 1

                        elif TestcaseLines[j].find("===") != -1:
                            if TestcaseLines[j-2].find("Desciption") != -1:
                                del TestcaseLines[j-2]
                                del TestcaseLines[j-2]
                                del TestcaseLines[j-2]
                                j = j - 2
                            else:
                                TestcaseLines[j-2] = "*" + TestcaseLines[j-2].strip() + "*"
                                del TestcaseLines[j]
                                j = j - 1

                        elif ((len(TestcaseLines[j].strip()) == 0) and (len(TestcaseLines[j-1].strip()) == 0)):
                            del TestcaseLines[j]
                            j = j - 1

                    j = j + 1
                
                f2 = open(TestcaseFile, "w")

                for Fileline in TestcaseLines:
                    f2.write(Fileline + "\n")

                f2.close()
                    
            else:
                DelTestcase = True

            if DelTestcase:
                del FileDataLines[i]
                i = i - 1
            else:
                Pos1 = FileDataLines[i].find("<tc-descriptions/")
                Pos2 = FileDataLines[i].find(">`")

                if ((Pos1 != -1) and (Pos2 != -1)):
                    if SetToctree:
                        SetToctree = False
                        FileDataLines.insert(i, "")
                        FileDataLines.insert(i, ".. toctree::")
                        FileDataLines.insert(i, "   ")
                        i = i + 3

                    FileDataLines[i] = "   " + FileDataLines[i][Pos1 + 17:Pos2]

        else:
            lastlength = length
            length = len(FileDataLines[i].strip())

            if (LenStart and (length > MaxLen)):
                MaxLen = length
 
            elif ((len(FileDataLines[i - 1].strip()) == 0) and (length == 0)):
                del FileDataLines[i]
                i = i - 1

        i = i + 1

    # last chapter
    if (ChapterNr == ChapterNrOld):
        if not LastChapterValid[ChapterNr]:
            LastPos1 = CounterStack.pop()
            LastPos2 = CounterStack.pop()

            for counter in range(LastPos1 - LastPos2):
                del FileDataLines[LastPos2]
                i = i - 1

            CounterStack.append(LastPos2)

    elif (ChapterNr < ChapterNrOld):
        counter = ChapterNrOld
        DeleteLast = True
        CurrentPos = CounterStack.pop()
                
        while (counter > ChapterNr):
            if (ChapterCounter[counter] > 0):
                DeleteLast = False
                        
                if ((counter == ChapterNrOld) and not LastChapterValid[ChapterNrOld]):
                    LastPos = CounterStack.pop()

                    for counter2 in range(CurrentPos - LastPos):
                        del FileDataLines[LastPos]
                        i = i - 1
                    CurrentPos = LastPos

            else:
                if DeleteLast:
                    LastPos = CounterStack.pop()

                    for counter2 in range(CurrentPos - LastPos):
                        del FileDataLines[LastPos]
                        i = i - 1
                    CurrentPos = LastPos

            ChapterCounter[counter] = 0
            counter = counter - 1

        if (DeleteLast and not LastChapterValid[ChapterNr]):
            LastPos2 = CounterStack.pop()

            for counter in range(CurrentPos - LastPos2):
                del FileDataLines[LastPos2]
                i = i - 1

            CounterStack.append(LastPos2)

    if ((MaxLen == 0) and SetToctree):
        counter = ChapterNr
        
        while (counter >= 0):
            if (ChapterCounter[counter] > 0):
                LastPos = CounterStack.pop()

                for k in range(i - LastPos):
                    del FileDataLines[LastPos]
                    i = i - 1

                break

            else:
                LastPos = CounterStack.pop()

                for k in range(i - LastPos):
                    del FileDataLines[LastPos]
                    i = i - 1
            counter = counter - 1
        print("{} --> False".format(CurChapterText)) 
    else:
        print("{} --> True".format(CurChapterText))

    f1 = open(SourcePath + "/source/list-of-testcase-chapters.rst", "w")
    for FileData in FileDataLines:
        f1.write(FileData + "\n")
    f1.close()

# --------------------------------------------------------------------
# check the file list-of-testcase-ids.rst
# --------------------------------------------------------------------

if os.path.isfile(SourcePath + "/source/list-of-testcase-ids.rst"):
    f1 = open(SourcePath + "/source/list-of-testcase-ids.rst", "r")
    ListOfTestcase = f1.readlines()
    ListOfTestcaseNew = []
    DelTestlineCounter = 0
        
    for Testcase in ListOfTestcase:
        Pos1 = Testcase.find("<")
        Pos2 = Testcase.find(">`")
           
        if ((Pos1 != -1) and (Pos2 != -1)):
            TestcaseFile = SourcePath + "/tc-descriptions/" + Testcase[Pos1 + 1:Pos2].strip() + ".rst"

            if os.path.isfile(TestcaseFile):
                DelTestlineCounter = 0
            else:
                DelTestlineCounter = 4

        if DelTestlineCounter == 0:
            ListOfTestcaseNew.append(Testcase)
        else:
            DelTestlineCounter = DelTestlineCounter - 1

    f1.close()

    f1 = open(SourcePath + "/source/list-of-testcase-ids.rst", "w")
    f1.writelines(ListOfTestcaseNew)
    f1.close()

# --------------------------------------------------------------------
# check the file list-of-testcase-list.rst
# --------------------------------------------------------------------

if os.path.isfile(SourcePath + "/source/list-of-testcase-list.rst"):
    f1 = open(SourcePath + "/source/list-of-testcase-list.rst", "r")

    Filedata = f1.read()
    FileDataLines = Filedata.split("\n")
    FileDataLinesNew = []

    f1.close()

    CounterStatus = -2
    i = 0
    MainMenue = -1
    MainMenueDefiniert = 0
    MainMenueImplementiert = 0
    GroupMenue = -1
    GroupMenueDefiniert = 0
    GroupMenueImplementiert = 0
    CurrentMenue = -1
        
    while (i < len(FileDataLines)):
        if (FileDataLines[i].find("   * - ") != -1):
            CounterStatus = 0
        
        elif CounterStatus > -1:
            if CounterStatus == 0:
                if (FileDataLines[i].find("======") != -1):
                    if MainMenue != -1:
                        FileDataLinesNew.append("   * - **Sum**:")
                        FileDataLinesNew.append("     - **" + str(MainMenueDefiniert) + "**")
                        FileDataLinesNew.append("     - **" + str(MainMenueImplementiert) + "**")
                        MainMenueDefiniert = 0
                        MainMenueImplementiert = 0

                    FileDataLinesNew.append("")
                    FileDataLinesNew.append(".. tabularcolumns:: |p{9.0cm}|p{3.0cm}|p{3.0cm}|")
                    FileDataLinesNew.append("")
                    FileDataLinesNew.append(".. list-table::")
                    FileDataLinesNew.append("")
                    FileDataLinesNew.append(FileDataLines[i - 1])
                    FileDataLinesNew.append("     - Definition")
                    FileDataLinesNew.append("     - Implementation")
                    MainMenue = len(FileDataLinesNew) - 1
                    CurrentMenue = 0

                elif (FileDataLines[i].find("------") != -1):
                    if GroupMenue != -1:
                        FileDataLinesNew[GroupMenue + 1] = "     - " + str(GroupMenueDefiniert)
                        FileDataLinesNew[GroupMenue + 2] = "     - " + str(GroupMenueImplementiert)
                        GroupMenueDefiniert = 0
                        GroupMenueImplementiert = 0

                    FileDataLinesNew.append(FileDataLines[i - 1])
                    GroupMenue = len(FileDataLinesNew) - 1
                    CurrentMenue = 1

                else:
                    CurrentMenue = -1

                CounterStatus = 1
                
            elif CounterStatus == 1:
                Testline = FileDataLines[i]
                j = Testline.find("Bestanden")
                counter = 0
                
                while j != -1:
                    counter = counter + 1
                    Testline = Testline[j + 9:]
                    j = Testline.find("Bestanden")

                MainMenueDefiniert = MainMenueDefiniert + counter
                MainMenueImplementiert = MainMenueImplementiert + counter
                GroupMenueDefiniert = GroupMenueDefiniert + counter
                GroupMenueImplementiert = GroupMenueImplementiert + counter

                Testline = FileDataLines[i]
                j = Testline.find("Nicht bestanden")
                counter = 0
                
                while j != -1:
                    counter = counter + 1
                    Testline = Testline[j + 14:]
                    j = Testline.find("Nicht bestanden")

                MainMenueDefiniert = MainMenueDefiniert + counter
                MainMenueImplementiert = MainMenueImplementiert + counter
                GroupMenueDefiniert = GroupMenueDefiniert + counter
                GroupMenueImplementiert = GroupMenueImplementiert + counter


                Testline = FileDataLines[i]
                j = Testline.find("Nicht ausgeführt")
                counter = 0
                
                while j != -1:
                    counter = counter + 1
                    Testline = Testline[j + 16:]
                    j = Testline.find("Nicht ausgeführt")

                MainMenueDefiniert = MainMenueDefiniert + counter
                GroupMenueDefiniert = GroupMenueDefiniert + counter

                if CurrentMenue > 0:
                    FileDataLinesNew.append("     - " + str(counter))
                    FileDataLinesNew.append("     - " + str(counter))

                i = i + 1
                CounterStatus = -1
                 
             
        else:
            FileDataLinesNew.append(FileDataLines[i])
        i = i + 1

    if MainMenue != -1:
        FileDataLinesNew.append("   * - **Sum**:")
        FileDataLinesNew.append("     - **" + str(MainMenueDefiniert) + "**")
        FileDataLinesNew.append("     - **" + str(MainMenueImplementiert) + "**")

    if GroupMenue != -1:
        FileDataLinesNew[GroupMenue + 1] = "     - " + str(GroupMenueDefiniert)
        FileDataLinesNew[GroupMenue + 2] = "     - " + str(GroupMenueImplementiert)

    f1 = open(SourcePath + "/source/list-of-testcase-list.rst", "w")
    for FileData in FileDataLinesNew:
        f1.write(FileData + "\n")
    f1.close()

# --------------------------------------------------------------------
# check the file list-of-testcase-ids.rst
# --------------------------------------------------------------------

if os.path.isfile(SourcePath + "/source/traceability-matrix.rst"):
    f1 = open(SourcePath + "/source/traceability-matrix.rst", "r")
    Filedata = f1.read()
    FileDataLines = Filedata.split("\n")
    FileDataLinesNew = []

    f1.close()
    j = 0

    while (j < len(FileDataLines)):
        if FileDataLines[j].find("TempList") != -1:
            TempListStart = j + 4
            break
        j = j + 1
        

    CopyNextLines = True
    i = 0

    while (i < len(FileDataLines)):
        pos1 = FileDataLines[i].find("   * - HE54")

        if (pos1 != -1):
            FindReq = FileDataLines[i][7:]
            j = TempListStart
            CopyNextLines = False
            TestcaseList = ""

            while (j < len(FileDataLines)):
                if (FileDataLines[j].find(FindReq + ",") != -1) or (re.search(FindReq + "\Z", FileDataLines[j]) != None):
                    pos1 = FileDataLines[j].find("*")
                    pos2 = FileDataLines[j].find(",")
                    if TestcaseList == "":
                        TestcaseList = "       * " + FileDataLines[j][pos1 + 1:pos2].strip()
                    else:
                        TestcaseList = TestcaseList + "\n      * " + FileDataLines[j][pos1 + 1:pos2].strip()
                j = j + 1

            FileDataLinesNew.append(FileDataLines[i])
            FileDataLinesNew.append("     - ")
            FileDataLinesNew.append("       " + TestcaseList)
            FileDataLinesNew.append("")
            print (FileDataLines[i], TestcaseList)
                    
        elif CopyNextLines:
            FileDataLinesNew.append(FileDataLines[i])
        i = i + 1

    f1 = open(SourcePath + "/source/traceability-matrix.rst", "w")
    for FileData in FileDataLinesNew:
        f1.write(FileData + "\n")
    f1.close()

# Generate the html- and pdf-document
BuildDirList = [SourcePath + "/source/", SourcePath + "/tc-descriptions/"]

# Instantiate multi builder. The last two params are optional.
builder = SphinxMultiBuilder(BuildDirList,
                             SourcePath + "/build/html",
                             SourcePath + "/Doc/html",
                             ["-b", "html", "-c", SourcePath + "/source"])
# build once
builder.build()

# start autobuilding on change in any input directory until ctrl+c is pressed.
builder.start_autobuilding()

builder = SphinxMultiBuilder(BuildDirList,
                             SourcePath + "/build/pdf",
                             SourcePath + "/Doc/pdf",
                             ["-M", "latexpdf", "-c", SourcePath + "/source"])
#build once
builder.build()

# start autobuilding on change in any input directory until ctrl+c is pressed.
builder.start_autobuilding()
