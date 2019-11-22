""" type: SofaContent """
import sys
import os
# all Paths
# all Modules:


def createScene(root):
    root.addObject('DefaultAnimationLoop', name='defaultAnimationLoop')
    root.addObject('DefaultVisualManagerLoop', name='defaultVisualManagerLoop')
    root.addObject('RequiredPlugin', name='NodePhysics')
    root.addObject('NodePhysics.MechanicalObject', name='state')
