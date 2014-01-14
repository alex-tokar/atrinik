'''
Implements objects such as game objects, map objects, archetypes, etc.
'''

from collections import OrderedDict, UserDict
import os

import system.constants


class AbstractObject:
    '''
    An abstract object, implementing properties shared by some other objects.
    '''
    
    def __init__(self, name):
        self.name = name
        self.attributes = OrderedDict()
        
    def setAttribute(self, attribute, value):
        '''Set object's attribute to value.'''
        self.attributes[attribute] = str(value)
        
    def getAttribute(self, attribute, default = None):
        '''Get object's attribute, return default if attribute doesn't exist.'''
        try:
            return self.attributes[attribute]
        except KeyError:
            return default
        
    def getAttributeInt(self, attribute):
        '''Get object's attribute as an integer.'''
        return int(self.getAttribute(attribute, 0))
        
    def setName(self, name):
        '''Change object's name.'''
        self.name = name
        
    def save(self):
        '''Save object's data as human-readable string.'''
        l = []
        
        for attribute in self.attributes:
            if attribute == "msg":
                l.append("msg\n{0}\nendmsg\n".format(self.attributes[attribute]))
            else:
                l.append("{0} {1}\n".format(attribute, self.attributes[attribute]))
            
        return "".join(l)
    
class AbstractObjectInventory(AbstractObject):
    '''Abstract object with inventory support functions.'''
    
    def __init__(self, *args):
        super().__init__(*args)
        
        # Links for parent object (if any) and inventory objects
        self.env = None
        self.inv = []
        
    def inventoryAdd(self, obj):
        '''Add an object to current object's inventory.'''
        self.inv.append(obj)
        
    def setParent(self, obj):
        '''Set this object's parent object.'''
        self.env = obj
        
    def getParentTop(self):
        ret = self
        
        while ret.env:
            ret = ret.env
            
        return ret

class GameObject(AbstractObjectInventory):
    '''Game object implementation.'''
    
    def __init__(self, *args):
        super().__init__(*args)
        
        self.map = None
        self.arch = None
        self.deleted = False
        
    def setArch(self, arch):
        self.arch = arch
        
    def getAttribute(self, attribute, default = None):
        val = super().getAttribute(attribute, default)
        
        if val == default and self.arch:
            val = self.arch.getAttribute(attribute, default)
            
        if val == default and attribute == "name":
            return self.name
        
        return val
    
    def isSameArchAttribute(self, attr):
        val1 = self.getAttribute(attr)
        val2 = self.arch.getAttribute(attr)
        
        if val1 == val2:
            return True
        
        try:
            if float(val1) == float(val2):
                return True
        except (ValueError, TypeError):
            pass
        
        return False
        
    @property 
    def x(self):
        '''Get X property of this object.'''
        return self.getAttributeInt("x")
        
    @property 
    def y(self):
        '''Get Y property of this object.'''
        return self.getAttributeInt("y")
    
    def delete(self):
        self.deleted = True
    
class MapObject(AbstractObject):
    '''Map object implementation.'''
    
    def __init__(self, *args):
        super(MapObject, self).__init__(*args)
        
        self.tiles = OrderedDict()
        
    def addObject(self, obj):
        '''Add object to the map.'''
        if not obj.x in self.tiles:
            self.tiles[obj.x] = OrderedDict()

        if not obj.y in self.tiles[obj.x]:
            self.tiles[obj.x][obj.y] = []

        self.tiles[obj.x][obj.y].append(obj)
        
    @property
    def width(self):
        '''Get map's width.'''
        return self.getAttributeInt("width")
    
    @property
    def height(self):
        '''Get map's height.'''
        return self.getAttributeInt("height")
    
    def isWorldMap(self):
        return self.getAttribute("name") == system.constants.game.world_map_name
    
class ArchObject(GameObject):
    '''Implements an archetype object.'''
    
    def __init__(self, *args):
        super(ArchObject, self).__init__(*args)
        
        self.head = None
        self.more = None
        
    def setHead(self, obj):
        self.head = obj
        
    def setMore(self, obj):
        '''Link a multi-part object.'''
        self.more = obj

class ArtifactObject(ArchObject):
    '''Implements artifact object.'''
    
    def setArtifactAttributes(self, attributes):
        '''Set artifact attributes.'''
        self.artifact_attributes = attributes
        
class RegionObject(AbstractObjectInventory):
    @property
    def parent(self):
        return self.getAttribute("parent")

class AbstractObjectCollection(UserDict):
    '''Implements a collection of objects.'''
    
    def __init__(self, name):
        super().__init__()
        
        self.name = name
        self.linked_collections = []
        self.path = None
        self.last_mtime = 0

    def get(self, key):
        '''
        Custom get implementation. This supports linked collections, so for
        example, the artifacts collection is linked to the archetypes
        collection, and if you use, for example, archetypes["amulet_copper"],
        it will first search the archetypes for the name 'amulet_copper',
        and if it's not found, it will search for the name in the artifacts
        collection. If the name is not found in any collection, None will be
        returned.
        '''
        
        try:
            return self[key]
        except KeyError:
            try:
                for collection in self.linked_collections:
                    return collection[key]
            except KeyError:
                pass
            
        return None
        
    def addLinkedCollection(self, collection):
        '''Links a collection to this one.'''
        self.linked_collections.append(collection)
    
    def setLastRead(self, path):
        '''Sets the time that the archetypes file was last parsed.'''
        self.last_read = os.path.getmtime(path)
        self.path = path

    def needReload(self, path):
        '''Checks if the collection needs to be reloaded.'''
        return self.path != path or os.path.getmtime(path) > self.last_read

class ArchObjectCollection(AbstractObjectCollection):
    '''Implements a collection of archetypes.'''
    pass

class ArtifactObjectCollection(AbstractObjectCollection):
    '''Implements a collection of artifacts.'''
    pass

class RegionObjectCollection(AbstractObjectCollection):
    '''Implements a collection of regions.'''
    pass