from maya import OpenMayaUI as omui
import PySide2
from PySide2.QtCore import *
from PySide2.QtGui import *
from PySide2.QtUiTools import *
from shiboken2 import wrapInstance
import pymel.core as pm
import pymel.core.datatypes as dt

#Användbar kod!

sourceRoot = pm.ls(sl=True)[0] #LÄGGER TILL SOURCE ROOT JOINTEN
targetRoot = pm.ls(sl=True)[0] #LÄGGER TILL TARGET ROOT JOINTEN
mySourceList = []
myTargetList = []
myRotationList = []
mySourceList.append(sourceRoot)
myTargetList.append(targetRoot)

##### PUTS SOURCE IN A LIST ####
def getJoints(node): #Denna åker igenom alla joints
    for child in node.getChildren():
       
        #pm.select(child)
        if child not in mySourceList:
                mySourceList.append(child)

        #pm.rotate(0, 50 , 0)
        if child.numChildren()>0:
            if child not in mySourceList:
                mySourceList.append(child)
                
        getJoints(child)                
        #pm.rotate(0, 50 , 0)   

getJoints(sourceRoot) #Does the things

##### PUTS TARGET IN A LIST ####
def getJoints(node): #Denna åker igenom alla joints
    for child in node.getChildren():
       
        #pm.select(child)
        if child not in myTargetList:
                myTargetList.append(child)

        #pm.rotate(0, 50 , 0)
        if child.numChildren()>0:
            if child not in myTargetList:
                myTargetList.append(child)
                
        getJoints(child)                
        #pm.rotate(0, 50 , 0)   
        
getJoints(targetRoot)

## mySourceList ## contains all joints in rig
## myTargetList ##

pm.select(myList)#VÄLJER ALLA JOINTS

####ROTATES A JOINT
pm.select(myList[4])
pm.rotate(0,0,0)

###### TRANSFER ORIGINAL POSE TO ONE BONE
pm.select(myList[19])
pm.rotate(myRotationList[19])

###### TRANSFER ORIGINAL POSE TO ALL BONES
for i, x in enumerate(myList):
    pm.select(x)
    pm.rotate(myRotationList[i])
    
###### GOES BACK TO THE ROOT
index = 4
tryParent = True

print (myList[index] + " is the current joint")


try:
    parent = myList[index].getParent()
    
except:
    print (myList[index] + " has no parent")
    
else:
    print (myList[index] + " has a parent: " + myList[index].getParent())
    
    
    
while tryParent == True:
    try:
       parent.getParent()
       
       if parent == root:
           print ( parent + " is the root")
           tryParent == False
           break
           
    except:
        tryParent = False
        print (parent + " has no parent")

    else:
        print (parent + " has a parent: " + parent.getParent())
        parent = parent.getParent()




#### FRAME STUFF ####
#Gets kurrent frame
ass = frame

listIndex = 2
##Sets current time
cmds.currentTime(0)

#GOING THROUGH SET AMOUT OF FRAMES
frameIndex = 30
for frame in range(frameIndex + 1):
    cmds.currentTime(frame)
    print "ass"
            

###### Test area 

test = mySourceList[1].getRotation().asMatrix()
print test.getRotation()

###### GET ALL JOINTS BINDING ROTATION MATIXES ######

##Matrix lists

    
#mySourceMatrixListOrientation = []
#mySourceMatrixROList = []

##Rotation List

##Orientation List
#for jnts in mySourceList:
#    mySourceMatrixListOrientation.append(jnts.getOrientation().asMatrix())

##Making the RO List For the source rig root
#for i, jnts in enumerate(mySourceList):
#	mySourceMatrixROList.append(mySourceMatrixListRotation[i] * mySourceMatrixListOrientation[i]) 
	
## mySourceMatrixROList ##


##### DO THE SAME WITH TARGET #####

##Rotation List

#myTargetMatrixListOrientation = []
#myTargetMatrixROList = []

##Rotation List


##Orientation List
#for jnts in myTargetList:
#    myTargetMatrixListOrientation.append(jnts.getOrientation().asMatrix())

##Making the RO List For the source rig root
#for i, jnts in enumerate(myTargetList):
#	myTargetMatrixROList.append(myTargetMatrixListRotation[i] * myTargetMatrixListOrientation[i]) 


## MAKE LISTS OF BIND POSE ROTATION MATRIXES ##
###########################
	
mySBindPoseRotationList = []
myTBindPoseRotationList = []

for jnts in mySourceList:
    mySBindPoseRotationList.append(jnts.getRotation().asMatrix())

for jnts in myTargetList:
    myTBindPoseRotationList.append(jnts.getRotation().asMatrix())
	
## myTBindPoseRotationList ##  ## BIND POSE ROTATION LISTS
## mySBindPoseRotationList ##

#############################

##### MAKE A LIST WITH INVERSED BINDPOSE ROTATION MATRIXES #####

mySBindPoseRotationInversedList = []
myTBindPoseRotationInversedList = []


for i, jnts in enumerate(mySourceList):
	mySBindPoseRotationInversedList.append(mySBindPoseRotationList[i].inverse())


for i, jnts in enumerate(myTargetList):
	myTBindPoseRotationInversedList.append(myTBindPoseRotationList[i].inverse())

## mySBindPoseRotationInversedList ## 
## myTBindPoseRotationInversedList ## 

#############################


### K (current joints  ("rotation matrix"  *  "rotation matrix inversed")  ON KEYFRAME) ###
mySKMatrixList = []
myTKMatrixList = []

### ISOLATED ROTATION LISTS ###
mySIsolatedRotationList = []
myTIsolatedRotationList = []

##CHECKS WHAT FRAMES THE JOINT HAS##
object = mySourceList[1]
attr = "tx"
myFramelist = cmds.keyframe(object + "." + attr, q=True)

##### TRY TO DO ALL THE WORK IN HERE, THIS RUNS EVERY FRAME #####
for frame in (myFramelist):
    cmds.currentTime(frame)
    ## Itterera igenom alla joints ##
    for i, x in enumerate(mySourceList):
    	## make a new list with the new joint parents ##
    	jointSParentList = []
    	jointTParentList = []
    	tParent = myTargetList[i].getParent()
    	sParent = mySourceList[i].getParent()
    	tryParent = True
    	
    	if mySourceList[i] == sourceRoot: ##Checks if joint is the root, if so, breakm the root does not have a parent.
    		break
    	else:
    		parent = mySourceList[i].getParent()
    		jointSParentList.append(parent) ##Else, append the joints parent to the parent list.
    		
    		while tryParent == True: ##If this is falce it means we have reached the root.
		    	try:
		    		parent.getParent() ##Gets the parents parent. 
		    		if parent == root:
		    			tryParent == False
		    			jointSParentList.append(parent)##This adds the root to the parent list as a last step before breaking.
		    			break
		    	except:
		    		tryParent = False ##If the joint did not have a parent then it is the root. 
		    	else:
		    		jointSParentList.append(parent) ##If it has a parent, append it to parent list. 
		    		parent = parent.getParent() ##This is now the new parent. 
		    
		    #T#################### Does the same thing as a above but with the target joint

    	tryParent = True
    	
    	if myTargetList[i] == sourceRoot:
    		break
    	else:
    	    parent = mySourceList[i].getParent()
    	    jointTParentList.append(parent) 
    	    while tryParent == True:
    	        print "aa"#
    	        try:
    	            print "SNÄLLA"
    	        
    	        try:
    	            print "aaa"
    	        
    	            print "aa"
    	             parent.getParent()
    	             if parent == root:
    	                 tryParent == False
    	                 jointTParentList.append(parent)
    	                 break
		           
		    except:
		    	tryParent = False
		       
		    else:
		    	jointTParentList.append(parent)
		    	parent = parent.getParent()
				    
				    	
					  ## jointSParentList ##
					  ## jointTParentList ##
		    ##Now we have gotten all the parents for the current joint
		    ##Next we should calculate the parent matrix somehow
		    sParentMatrix = 1;
		    tParentMatrix = 1;
		    
		     for jnts in (jointSParentList):
		     	sParentMatrix = jnts.getRotation().asMatrix() * sParentMatrix
		     	
		     for jnts in (jointTParentList):
		     	tParentMatrix = jnts.getRotation().asMatrix() * tParentMatrix
		     	
		    ##If we are lucky this made 2 parent matrixes
		    ## sParentMatrix ##
		    ## tParentMatrix ##
		    
		    ##Now we should get the rotation for the joint
		    
		    sJointRotation = mySourceList[i].getRotation().asMatrix()
		    tJointRotation = myTargetList[i].getRotation().asMatrix()
		    
		    ##K (current joints  ("rotation matrix"  *  "rotation matrix inversed")  ON KEYFRAME)
		    
		    mySKMatrix = mySBindPoseRotationList[i] * mySBindPoseRotationInversedList[i] ## isolatedRotation
		    myTKMatrix = myTBindPoseRotationList[i] * myTBindPoseRotationInversedList[i] ##I dont think this one is needed
		    
		    ##Things for world Space rotation calculation
		    sOrientationInversed = (mySourceList[i].getOrientation().asMatrix()).inverse()
		    sParentsInversed = sParentMatrix.inversse()
		    isolatedRotation = mySKMatrixList
		    sParents = sParentMatrix
		    sOrientation = mySourceList[i].getOrientation().asMatrix()
		    
		    worldSpaceRotation = (sOrientationInversed * sParentsInversed * isolatedRotation * sParents * sOrientation)
		    
		    ##Things for translatedRotation calculation.
		    tOrientation = myTargetList[i].getOrientation().asMatrix()
		    tParents = tParentMatrix
		    #worldSpaceRotation
		    tParentsInversed = tParent;atrix.inversed()
		    tOrientationInversed = tOrientation.inversed()
		    
		    translatedRotation = (tOrientation * tParents * worldSpaceRotation * tParentsInversed * tOrientationInversed)
		    
		    ## FINAL ROTATION MATRIX ##
		    
		    finalRotationMatrix = (myTBindPoseRotationList * translatedRotation)
		    
		    ##Calculate final rotation from matrix to vector
		    finalVectorRotation = finalRotationMatrix.dt.EulerRotation(tJointRotation) 
		    finalVectorRotation = dt.degrees(finalVectorRotation) ##Kolla igenom detta mer
		    

    



#####



#####

##### KONVERTERA JOIN BAS TILL PARENT JOINT BAS #####

##START MATRIXES FOR SOURCE
cmds.currentTime(0)
SBindPose = mySourceList[listIndex] ##BIND POSE ROTATION
sk0 = (SBindPose.getRotation().AsMatrix() * SBindPose.getOrientation().AsMatrix()) 
sk = (mySourceList[listIndex].getRotation().AsMatrix() * mySourceList[listIndex].getOrientation().AsMatrix()) ##KEY FRAME ROTATION
sParentMatrix = 0 ##Parent matrix skall innehålla alla tidigare matriser

##START MATRIXES FOR TARGET
cmds.currentTime(0)
TBindPose = myTargetList[listIndex]
tk0 = (TBindPose.getRotation().AsMatrix() * TBindPose.getOrientation().AsMatrix()) 
tk = (myTargetList[listIndex].getRotation().AsMatrix() * myTargetList[listIndex].getOrientation().AsMatrix()) ##KEY FRAME ROTATION
tParentMatrix = 0

#ISOLERA MATRISEN
#sBindPoseInversed * sRotation = isolatedRotation
##Denna skall innehålla alla tidigare matriser samt K0 och den skall vara inversed
##KEY FRAME ROTATION
## is the inverse of the rotation matrix of a source joint on the first frame of the animation
sBindPoseInversed = ((SBindPose.getRotation().asMatrix() * SBindPose.getOrientation().asMatrix()) * k0).inverse() ##BIND POSE INVERS MATRIX

##Denna ksall innehålla alla tidigare matriser samt K ## is the rotation of the same source joint at keyframe X
sRotation = (((SBindPose.getRotation().asMatrix() * SBindPose.getOrientation().asMatrix()) * k) ##ROTATION KEY FRAME MATRIX

isolatedRotation = sBindPoseInversed * sRotation #Hopefully gets the isolated rotation

##WORLD SPACE ROTATION
## sOrientationInversed * sParentInversed * isolatedRotation * sParents * sOrientation = worldSpaceRotation
sOrientation = (SBindPose.getRotation().asMatrix() * SBindPose.getOrientation().asMatrix())
sParents = parentMatrix
isolatedRotation = isolatedRotation
sParentInversed = sParentMatrix.inversed()
sOrientationInversed = sOrientation.inversed()

worldSpaceRotation = (sOrientationInversed * sParentInversed * isolatedRotation * sParents * sOrientation)


######## SOURCE RIG TRANSLATED ROTATION #########

tParentsInversed = tParentMatrix.inversed()
tOrientationInversed = ((tBindPose.getRotation().asMatrix() * tBindPose.getOrientation().asMatrix()) * sk0).inversed()
tOrientation = ((tBindPose.getRotation().asMatrix() * tBindPose.getOrientation().asMatrix()) * sk)
worldSpaceRotation = worldSpaseRotation
tParents = tParentMatrix


translatedRotation (tOrientation * tParents * worldSpaceRotation * tParentsInversed * tOrientationInversed)

##FINAL ROTATION

finalRotation = TBindPose * translatedRotation