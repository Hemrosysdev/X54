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
            print( CurChapterText + " --> " + str(LastChapterValid[ChapterNr]))
            
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
                        
                            #if len(Text) > 52:
                            #    Text = Text[:45] + " ..."
                            
                            TestcaseLines[j-1] = Text
                            TestcaseLines[j] = replaceData("=", len(TestcaseLines[j-1]))

                        elif TestcaseLines[j].find("Nicht ausgef") != -1:
                            DelTestcase = True
                            break
                    
                        elif TestcaseLines[j].find("**Anlagen**") != -1:
                            TestcaseLines.insert(j + 1, "")
                            TestcaseLines.insert(j, "")
                            j = j + 2
                            AnlagenStart = j + 1
                            ReadStatus = 1

                        else:
                            TestcaseLines[j] = TestcaseLines[j].replace("_", " ")

                    elif (ReadStatus == 1):
                        if TestcaseLines[j].find("**Beschreibung**") != -1:
                            TestcaseLines.insert(j, "")
                            j = j + 1
                            ReadStatus = 2

                        elif len(TestcaseLines[j].strip()) == 0:
                            del TestcaseLines[j]
                            j = j - 1

                        else:
                            TestcaseLines[j] = "- " + TestcaseLines[j].replace(",", "")

                    elif (ReadStatus == 2):
                        if TestcaseLines[j].find("--------") != -1:
                            if ((TestcaseLines[j-2].find("Test-Beschreibung") != -1) or (TestcaseLines[j-2].find("Testbeschreibung") != -1)):
                                del TestcaseLines[j-2]
                                del TestcaseLines[j-2]
                                del TestcaseLines[j-2]
                                j = j - 2
                            else:
                                TestcaseLines[j - 2] = "*" + TestcaseLines[j - 2].strip() + "*"
                                del TestcaseLines[j]
                                j = j - 1

                        elif TestcaseLines[j].find("===") != -1:
                            if TestcaseLines[j-2].find("Beschreibung") != -1:
                                del TestcaseLines[j-2]
                                del TestcaseLines[j-2]
                                del TestcaseLines[j-2]
                                j = j - 2
                            else:
                                TestcaseLines[j-2] = "*" + TestcaseLines[j-2].strip() + "*"
                                del TestcaseLines[j]
                                j = j - 1

                        elif TestcaseLines[j].find("**Report**") != -1:
                            TestcaseLines.insert(j + 1, "")
                            TestcaseLines.insert(j + 1, "::")
                            TestcaseLines.insert(j + 1, "")
                            j = j + 3
                        
                            ReadStatus = 3

                        elif ((len(TestcaseLines[j].strip()) == 0) and (len(TestcaseLines[j-1].strip()) == 0)):
                            del TestcaseLines[j]
                            j = j - 1

                        else:
                            TestcaseLines[j] = TestcaseLines[j].replace(",", "")

                    else:
                        if (len(TestcaseLines[j].strip()) == 0):
                            del TestcaseLines[j]
                            j = j - 1

                        elif TestcaseLines[j].find("For full console") != -1:
                            del TestcaseLines[j]
                            j = j - 1 

                        elif TestcaseLines[j].find("rest of report skipped - see") != -1:
                            TestcaseLines[j] = "   rest of report skipped - see ...console.log"                          

                        elif TestcaseLines[j].find(".log") != -1:
                            Text = TestcaseLines[j]
                            del TestcaseLines[j]
                            TestcaseLines.insert(AnlagenStart, "- " + Text)

                        else:
                            TestcaseLines[j] = "   " + TestcaseLines[j]

                    j = j + 1
                
                if DelTestcase:
                    os.remove(TestcaseFile)
                else:
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
                    TcNr = TcNr + 1

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
        print( CurChapterText + " --> False") 
    else:
        print( CurChapterText + " --> True") 

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
# check the file list-of-testcase-results.rst
# --------------------------------------------------------------------

if os.path.isfile(SourcePath + "/source/list-of-testcase-result.rst"):
    f1 = open(SourcePath + "/source/list-of-testcase-result.rst", "r")

    Filedata = f1.read()
    FileDataLines = Filedata.split("\n")
    FileDataLinesNew = []

    f1.close()

    CopyNextLines = True
    CounterStatus = -2
    i = 0
    MainMenue = -1
    MainMenuePASS = 0
    MainMenueFAIL = 0
    GroupMenue = -1
    GroupMenuePASS = 0
    GroupMenueFAIL = 0
    CurrentMenue = -1
    LastEntry = -1
        
    while (i < len(FileDataLines)):
        Pos1 = FileDataLines[i].find("<")
        Pos2 = FileDataLines[i].find(">`")
           
        if ((Pos1 != -1) and (Pos2 != -1)):
            CopyNextLines = False
            CounterStatus = -1
            TestcaseFile = SourcePath + "/tc-descriptions/" + FileDataLines[i][Pos1 + 1:Pos2].strip() + ".rst"

            if os.path.isfile(TestcaseFile):
                if FileDataLines[i].find("Nicht bestanden") != -1:
                    NewText = FileDataLines[i].replace("Nicht bestanden", "FAIL")
                else:
                    NewText = FileDataLines[i].replace("Bestanden", "PASS")
                FileDataLinesNew.append(NewText)
                FileDataLinesNew.append(FileDataLines[i + 1])

            i = i + 1

        elif (FileDataLines[i].find("   * - ") != -1):
            CounterStatus = 0
        
        elif CounterStatus > -1:
            if CounterStatus == 0:
                if (FileDataLines[i].find("======") != -1):
                    if MainMenue != -1:
                        FileDataLinesNew.append("   * - **Sum**:")
                        FileDataLinesNew.append("     - **" + str(MainMenuePASS) + "**")
                        FileDataLinesNew.append("     - **" + str(MainMenueFAIL) + "**")
                        MainMenuePASS = 0
                        MainMenueFAIL = 0

                    FileDataLinesNew.append("")
                    FileDataLinesNew.append(".. tabularcolumns:: |p{13.5cm}|p{1.0cm}|p{1.0cm}|")
                    FileDataLinesNew.append("")
                    FileDataLinesNew.append(".. list-table::")
                    FileDataLinesNew.append("")
                    FileDataLinesNew.append(FileDataLines[i - 1])
                    FileDataLinesNew.append("     - PASS")
                    FileDataLinesNew.append("     - FAIL")
                    MainMenue = len(FileDataLinesNew) - 1
                    #GroupMenue = -1
                    #LastEntry = -1
                    CurrentMenue = 0

                elif (FileDataLines[i].find("------") != -1):
                    if GroupMenue != -1:
                        FileDataLinesNew[GroupMenue + 1] = "     - " + str(GroupMenuePASS)
                        FileDataLinesNew[GroupMenue + 2] = "     - " + str(GroupMenueFAIL)
                        GroupMenuePASS = 0
                        GroupMenueFAIL = 0

                    FileDataLinesNew.append(FileDataLines[i - 1])
                    GroupMenue = len(FileDataLinesNew) - 1
                    LastEntry = len(FileDataLinesNew) - 1
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

                if CurrentMenue > 0:
                    FileDataLinesNew.append("     - " + str(counter))

                MainMenuePASS = MainMenuePASS + counter
                GroupMenuePASS = GroupMenuePASS + counter
                CounterStatus = 2
                
            elif CounterStatus == 2:
                Testline = FileDataLines[i]
                j = Testline.find("Nicht bestanden")
                counter = 0
                
                while j != -1:
                    counter = counter + 1
                    Testline = Testline[j + 14:]
                    j = Testline.find("Nicht bestanden")

                if CurrentMenue > 0:
                    FileDataLinesNew.append("     - " + str(counter))

                MainMenueFAIL = MainMenueFAIL + counter
                GroupMenueFAIL = GroupMenueFAIL + counter
                CounterStatus = -1
                 
             
        elif CopyNextLines:
            CopyNextLine = True
            FileDataLinesNew.append(FileDataLines[i])
        i = i + 1

    if LastEntry != -1:
        FileDataLinesNew.insert(LastEntry + 3, "     - **" + str(MainMenueFAIL) + "**")
        FileDataLinesNew.insert(LastEntry + 3, "     - **" + str(MainMenuePASS) + "**")
        FileDataLinesNew.insert(LastEntry + 3, "   * - **Sum**:")
    else:
        FileDataLinesNew.insert(MainMenue + 1, "     - **" + str(MainMenueFAIL) + "**")
        FileDataLinesNew.insert(MainMenue + 1, "     - **" + str(MainMenuePASS) + "**")
        FileDataLinesNew.insert(MainMenue + 1, "   * - **Sum**:")

    if GroupMenue != -1:
        FileDataLinesNew[GroupMenue + 1] = "     - " + str(GroupMenuePASS)
        FileDataLinesNew[GroupMenue + 2] = "     - " + str(GroupMenueFAIL)

    f1 = open(SourcePath + "/source/list-of-testcase-result.rst", "w")
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
            PassCounter = 0
            FailCounter = 0
            SkipCounter = 0
            CopyNextLines = False

            while (j < len(FileDataLines)):
                if (FileDataLines[j].find(FindReq + ",") != -1) or (re.search(FindReq + "\Z", FileDataLines[j]) != None):
                    if FileDataLines[j].find("Nicht ausgeführt") != -1:
                        SkipCounter = SkipCounter + 1
                    elif FileDataLines[j].find("Bestanden") != -1:
                        PassCounter = PassCounter + 1
                    else:
                        FailCounter = FailCounter + 1
                j = j + 1

            FileDataLinesNew.append(FileDataLines[i])
            FileDataLinesNew.append("     - " + str(PassCounter))
            FileDataLinesNew.append("     - " + str(FailCounter))
            FileDataLinesNew.append("     - " + str(SkipCounter))
            FileDataLinesNew.append("")
            print( FileDataLines[i], PassCounter, FailCounter, SkipCounter)
                    
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
