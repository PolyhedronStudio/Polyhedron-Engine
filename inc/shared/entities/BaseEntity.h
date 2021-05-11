#ifndef __SHARED_ENTITIES_BASEENTITY_H__
#define __SHARED_ENTITIES_BASEENTITY_H__

class BaseEntity {
public:
    EntityState  state;
    struct gclient_s* client;    // NULL if not a player
                                    // the server expects the first part
                                    // of gclient_s to be a PlayerState
                                    // but the rest of it is opaque

    qboolean    inUse;
    int         linkCount;

    // FIXME: move these fields to a server private sv_entity_t
    list_t      area;               // linked to a division node or leaf

    int         numClusters;       // if -1, use headNode instead
    int         clusterNumbers[MAX_ENT_CLUSTERS];
    int         headNode;           // unused if numClusters != -1
    int         areaNumber, areaNumber2;

    //================================

    int         serverFlags;
    vec3_t      mins, maxs;
    vec3_t      absMin, absMax, size;
    uint32_t    solid;
    int         clipMask;
    BaseEntity  *owner;


    // DO NOT MODIFY ANYTHING ABOVE THIS, THE SERVER
    // EXPECTS THE FIELDS IN THAT ORDER!
};

#endif