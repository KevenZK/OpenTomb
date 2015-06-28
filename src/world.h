
#ifndef WORLD_H
#define WORLD_H

#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_opengl.h>
#include <cstdint>
#include "audio.h"
#include "camera.h"
#include "bordered_texture_atlas.h"
#include <bullet/LinearMath/btScalar.h>
#include <bullet/LinearMath/btVector3.h>
#include "object.h"

#include <memory>
#include <vector>

// Native TR floor data functions

#define TR_FD_FUNC_PORTALSECTOR                 0x01
#define TR_FD_FUNC_FLOORSLANT                   0x02
#define TR_FD_FUNC_CEILINGSLANT                 0x03
#define TR_FD_FUNC_TRIGGER                      0x04
#define TR_FD_FUNC_DEATH                        0x05
#define TR_FD_FUNC_CLIMB                        0x06
#define TR_FD_FUNC_FLOORTRIANGLE_NW             0x07    //  [_\_]
#define TR_FD_FUNC_FLOORTRIANGLE_NE             0x08    //  [_/_]
#define TR_FD_FUNC_CEILINGTRIANGLE_NW           0x09    //  [_/_]
#define TR_FD_FUNC_CEILINGTRIANGLE_NE           0x0A    //  [_\_]
#define TR_FD_FUNC_FLOORTRIANGLE_NW_PORTAL_SW   0x0B    //  [P\_]
#define TR_FD_FUNC_FLOORTRIANGLE_NW_PORTAL_NE   0x0C    //  [_\P]
#define TR_FD_FUNC_FLOORTRIANGLE_NE_PORTAL_SE   0x0D    //  [_/P]
#define TR_FD_FUNC_FLOORTRIANGLE_NE_PORTAL_NW   0x0E    //  [P/_]
#define TR_FD_FUNC_CEILINGTRIANGLE_NW_PORTAL_SW 0x0F    //  [P\_]
#define TR_FD_FUNC_CEILINGTRIANGLE_NW_PORTAL_NE 0x10    //  [_\P]
#define TR_FD_FUNC_CEILINGTRIANGLE_NE_PORTAL_NW 0x11    //  [P/_]
#define TR_FD_FUNC_CEILINGTRIANGLE_NE_PORTAL_SE 0x12    //  [_/P]
#define TR_FD_FUNC_MONKEY                       0x13
#define TR_FD_FUNC_MINECART_LEFT                0x14    // In TR3 only. Function changed in TR4+.
#define TR_FD_FUNC_MINECART_RIGHT               0x15    // In TR3 only. Function changed in TR4+.

// Native TR trigger (TR_FD_FUNC_TRIGGER) types.

#define TR_FD_TRIGTYPE_TRIGGER          0x00    // If Lara is in sector, run (any case).
#define TR_FD_TRIGTYPE_PAD              0x01    // If Lara is in sector, run (land case).
#define TR_FD_TRIGTYPE_SWITCH           0x02    // If item is activated, run, else stop.
#define TR_FD_TRIGTYPE_KEY              0x03    // If item is activated, run.
#define TR_FD_TRIGTYPE_PICKUP           0x04    // If item is picked up, run.
#define TR_FD_TRIGTYPE_HEAVY            0x05    // If item is in sector, run, else stop.
#define TR_FD_TRIGTYPE_ANTIPAD          0x06    // If Lara is in sector, stop (land case).
#define TR_FD_TRIGTYPE_COMBAT           0x07    // If Lara is in combat state, run (any case).
#define TR_FD_TRIGTYPE_DUMMY            0x08    // If Lara is in sector, run (air case).
#define TR_FD_TRIGTYPE_ANTITRIGGER      0x09    // TR2-5 only: If Lara is in sector, stop (any case).
#define TR_FD_TRIGTYPE_HEAVYSWITCH      0x0A    // TR3-5 only: If item is activated by item, run.
#define TR_FD_TRIGTYPE_HEAVYANTITRIGGER 0x0B    // TR3-5 only: If item is activated by item, stop.
#define TR_FD_TRIGTYPE_MONKEY           0x0C    // TR3-5 only: If Lara is monkey-swinging, run.
#define TR_FD_TRIGTYPE_SKELETON         0x0D    // TR5 only: Activated by skeleton only?
#define TR_FD_TRIGTYPE_TIGHTROPE        0x0E    // TR5 only: If Lara is on tightrope, run.
#define TR_FD_TRIGTYPE_CRAWLDUCK        0x0F    // TR5 only: If Lara is crawling, run.
#define TR_FD_TRIGTYPE_CLIMB            0x10    // TR5 only: If Lara is climbing, run.

// Native trigger function types.

#define TR_FD_TRIGFUNC_OBJECT           0x00
#define TR_FD_TRIGFUNC_CAMERATARGET     0x01
#define TR_FD_TRIGFUNC_UWCURRENT        0x02
#define TR_FD_TRIGFUNC_FLIPMAP          0x03
#define TR_FD_TRIGFUNC_FLIPON           0x04
#define TR_FD_TRIGFUNC_FLIPOFF          0x05
#define TR_FD_TRIGFUNC_LOOKAT           0x06
#define TR_FD_TRIGFUNC_ENDLEVEL         0x07
#define TR_FD_TRIGFUNC_PLAYTRACK        0x08
#define TR_FD_TRIGFUNC_FLIPEFFECT       0x09
#define TR_FD_TRIGFUNC_SECRET           0x0A
#define TR_FD_TRIGFUNC_CLEARBODIES      0x0B    // Unused in TR4
#define TR_FD_TRIGFUNC_FLYBY            0x0C
#define TR_FD_TRIGFUNC_CUTSCENE         0x0D

// Action type specifies a kind of action which trigger performs. Mostly
// it's only related to item activation, as any other trigger operations
// are not affected by action type in original engines.

#define TR_ACTIONTYPE_NORMAL  0
#define TR_ACTIONTYPE_ANTI    1
#define TR_ACTIONTYPE_SWITCH  2
#define TR_ACTIONTYPE_BYPASS -1 // Used for "dummy" triggers from originals.

// Activator specifies a kind of triggering event (NOT to be confused
// with activator type mentioned below) to occur, like ordinary trigger,
// triggering by inserting a key, turning a switch or picking up item.

#define TR_ACTIVATOR_NORMAL 0
#define TR_ACTIVATOR_SWITCH 1
#define TR_ACTIVATOR_KEY    2
#define TR_ACTIVATOR_PICKUP 3

// Activator type is used to identify activator kind for specific
// trigger types (so-called HEAVY triggers). HEAVY means that trigger
// is activated by some other item, rather than Lara herself.

#define TR_ACTIVATORTYPE_LARA 0
#define TR_ACTIVATORTYPE_MISC 1

// Various room flags specify various room options. Mostly, they
// specify environment type and some additional actions which should
// be performed in such rooms.

#define TR_ROOM_FLAG_WATER          0x0001
#define TR_ROOM_FLAG_QUICKSAND      0x0002  // Moved from 0x0080 to avoid confusion with NL.
#define TR_ROOM_FLAG_SKYBOX         0x0008
#define TR_ROOM_FLAG_UNKNOWN1       0x0010
#define TR_ROOM_FLAG_WIND           0x0020
#define TR_ROOM_FLAG_UNKNOWN2       0x0040  ///@FIXME: Find what it means!!! Always set by Dxtre3d.
#define TR_ROOM_FLAG_NO_LENSFLARE   0x0080  // In TR4-5. Was quicksand in TR3.
#define TR_ROOM_FLAG_MIST           0x0100  ///@FIXME: Unknown meaning in TR1!!!
#define TR_ROOM_FLAG_CAUSTICS       0x0200
#define TR_ROOM_FLAG_UNKNOWN3       0x0400
#define TR_ROOM_FLAG_DAMAGE         0x0800  ///@FIXME: Is it really damage (D)?
#define TR_ROOM_FLAG_POISON         0x1000  ///@FIXME: Is it really poison (P)?

//Room light mode flags (TR2 ONLY)

#define TR_ROOM_LIGHTMODE_FLICKER   0x1

// Sector flags specify various unique sector properties.
// Derived from native TR floordata functions.

#define SECTOR_FLAG_CLIMB_NORTH     0x00000001  // subfunction 0x01
#define SECTOR_FLAG_CLIMB_EAST      0x00000002  // subfunction 0x02
#define SECTOR_FLAG_CLIMB_SOUTH     0x00000004  // subfunction 0x04
#define SECTOR_FLAG_CLIMB_WEST      0x00000008  // subfunction 0x08
#define SECTOR_FLAG_CLIMB_CEILING   0x00000010
#define SECTOR_FLAG_MINECART_LEFT   0x00000020
#define SECTOR_FLAG_MINECART_RIGHT  0x00000040
#define SECTOR_FLAG_TRIGGERER_MARK  0x00000080
#define SECTOR_FLAG_BEETLE_MARK     0x00000100
#define SECTOR_FLAG_DEATH           0x00000200

// Sector material specifies audio response from character footsteps, as well as
// footstep texture option, plus possible vehicle physics difference in the future.

#define SECTOR_MATERIAL_MUD         0   // Classic one, TR1-2.
#define SECTOR_MATERIAL_SNOW        1
#define SECTOR_MATERIAL_SAND        2
#define SECTOR_MATERIAL_GRAVEL      3
#define SECTOR_MATERIAL_ICE         4
#define SECTOR_MATERIAL_WATER       5
#define SECTOR_MATERIAL_STONE       6
#define SECTOR_MATERIAL_WOOD        7
#define SECTOR_MATERIAL_METAL       8
#define SECTOR_MATERIAL_MARBLE      9
#define SECTOR_MATERIAL_GRASS       10
#define SECTOR_MATERIAL_CONCRETE    11
#define SECTOR_MATERIAL_OLDWOOD     12
#define SECTOR_MATERIAL_OLDMETAL    13

// Maximum number of flipmaps specifies how many flipmap indices to store. Usually,
// TR1-3 doesn't contain flipmaps above 10, while in TR4-5 number of flipmaps could
// be as much as 14-16. To make sure flipmap array will be suitable for all game
// versions, it is set to 32.

#define FLIPMAP_MAX_NUMBER          32

// Activation mask operation can be either XOR (for switch triggers) or OR (for any
// other types of triggers).

#define AMASK_OP_OR  0
#define AMASK_OP_XOR 1

class btCollisionShape;
class btRigidBody;

struct Room;
struct Polygon;
struct Camera;
struct Portal;
struct Render;
struct Frustum;
struct BaseMesh;
struct StaticMesh;
struct Entity;
struct SkeletalModel;
struct RedBlackHeader_s;
struct SSBoneFrame;


struct BaseItem
{
    uint32_t                    id;
    uint32_t                    world_model_id;
    uint16_t                    type;
    uint16_t                    count;
    char                        name[64];
    std::unique_ptr<SSBoneFrame> bf;

    ~BaseItem();
};

struct RoomBox
{
    int32_t     x_min;
    int32_t     x_max;
    int32_t     y_min;
    int32_t     y_max;
    int32_t     true_floor;
    int32_t     overlap_index;
};

struct RoomSector
{
    uint32_t                    trig_index; // Trigger function index.
    int32_t                     box_index;

    uint32_t                    flags;      // Climbability, death etc.
    uint32_t                    material;   // Footstep sound and footsteps.

    int32_t                     floor;
    int32_t                     ceiling;

    RoomSector        *sector_below;
    RoomSector        *sector_above;
    std::shared_ptr<Room> owner_room;    // Room that contain this sector

    int16_t                     index_x;
    int16_t                     index_y;
    btVector3 pos;

    btVector3                   ceiling_corners[4];
    uint8_t                     ceiling_diagonal_type;
    uint8_t                     ceiling_penetration_config;

    btVector3                   floor_corners[4];
    uint8_t                     floor_diagonal_type;
    uint8_t                     floor_penetration_config;

    int32_t                     portal_to_room;
};


struct SectorTween
{
    btVector3                   floor_corners[4];
    uint8_t                     floor_tween_type;

    btVector3                   ceiling_corners[4];
    uint8_t                     ceiling_tween_type;
};

struct Sprite;

struct RoomSprite
{
    Sprite             *sprite;
    btVector3 pos;
    bool was_rendered;
};

struct EngineContainer;
struct SpriteBuffer;
struct Light;
struct AnimSeq;

struct Room : public Object
{
    uint32_t                    id;                                             // room's ID
    uint32_t                    flags;                                          // room's type + water, wind info
    int16_t                     light_mode;                                     // (present only in TR2: 0 is normal, 1 is flickering(?), 2 and 3 are uncertain)
    uint8_t                     reverb_info;                                    // room reverb type
    uint8_t                     water_scheme;
    uint8_t                     alternate_group;

    bool active;                                         // flag: is active
    bool is_in_r_list;                                   // is room in render list
    bool hide;                                           // do not render
    std::shared_ptr<BaseMesh> mesh;                                           // room's base mesh
    //struct bsp_node_s          *bsp_root;                                       // transparency polygons tree; next: add bsp_tree class as a bsp_tree header
    SpriteBuffer *sprite_buffer;               // Render data for sprites

    std::vector<std::shared_ptr<StaticMesh>> static_mesh;
    std::vector<RoomSprite> sprites;

    std::vector<std::shared_ptr<EngineContainer>> containers;                                     // engine containers with moveables objects

    btVector3 bb_min;                                      // room's bounding box
    btVector3 bb_max;                                      // room's bounding box
    btTransform transform;                                  // GL transformation matrix
    btScalar                    ambient_lighting[3];

    std::vector<Light> lights;

    std::vector<Portal> portals;                                        // room portals array
    std::shared_ptr<Room> alternate_room;                                 // alternative room pointer
    std::shared_ptr<Room> base_room;                                      // base room == room->alternate_room->base_room

    uint16_t                    sectors_x;
    uint16_t                    sectors_y;
    std::vector<RoomSector> sectors;

    uint16_t                    active_frustums;                                // current number of this room active frustums
    std::vector<std::shared_ptr<Frustum>> frustum;
    uint16_t                    max_path;                                       // maximum number of portals from camera to this room

    uint16_t                    near_room_list_size;
    std::shared_ptr<Room> near_room_list[32];
    uint16_t                    overlapped_room_list_size;
    std::shared_ptr<Room> overlapped_room_list[32];
    btRigidBody                *bt_body;

    std::unique_ptr<EngineContainer> self;
};


struct World
{
    char                       *name;
    uint32_t                    id;
    uint32_t                    version;

    std::vector< std::shared_ptr<Room> > rooms;

    std::vector<RoomBox> room_boxes;

    uint32_t                    flip_count;             // Number of flips
    uint8_t                    *flip_map;               // Flipped room activity array.
    uint8_t                    *flip_state;             // Flipped room state array.

    BorderedTextureAtlas     *tex_atlas;
    std::vector<GLuint> textures;               // OpenGL textures indexes

    std::vector<AnimSeq> anim_sequences;         // Animated textures

    std::vector<std::shared_ptr<BaseMesh>> meshes;                 // Base meshes data

    std::vector<Sprite> sprites;                // Base sprites data

    std::vector<SkeletalModel> skeletal_models;        // base skeletal models data

    std::shared_ptr<Entity>   Character;              // this is an unique Lara's pointer =)
    SkeletalModel    *sky_box;                // global skybox

    std::map<uint32_t, std::shared_ptr<Entity> > entity_tree;            // tree of world active objects
    std::map<uint32_t, std::shared_ptr<BaseItem> > items_tree;             // tree of world items

    uint32_t                    type;

    std::vector<StatCameraSink> cameras_sinks;          // Cameras and sinks.

    std::vector<int16_t> anim_commands;

    std::vector<AudioEmitter> audio_emitters;         // Audio emitters.
    std::vector<int16_t> audio_map;              // Effect indexes.
    std::vector<AudioEffect> audio_effects;          // Effects and their parameters.

    std::vector<ALuint> audio_buffers;          // Samples.
    std::vector<AudioSource> audio_sources;          // Channels.
    std::vector<StreamTrack> stream_tracks;          // Stream tracks.
    std::vector<uint8_t> stream_track_map;       // Stream track flag map.

    void updateAnimTextures();
    void calculateWaterTint(std::array<float,4> *tint, bool fixed_colour);
};

void World_Prepare(World* world);
void World_Empty(World* world);
int compEntityEQ(void *x, void *y);
int compEntityLT(void *x, void *y);
uint32_t World_SpawnEntity(uint32_t model_id, uint32_t room_id, const btVector3 *pos, const btVector3 *ang, int32_t id);
std::shared_ptr<Entity> World_GetEntityByID(World* world, uint32_t id);
std::shared_ptr<BaseItem> World_GetBaseItemByID(World* world, uint32_t id);

void Room_Empty(std::shared_ptr<Room> room);
void Room_AddEntity(std::shared_ptr<Room> room, std::shared_ptr<Entity> entity);
bool Room_RemoveEntity(std::shared_ptr<Room> room, std::shared_ptr<Entity> entity);

void Room_AddToNearRoomsList(std::shared_ptr<Room> room, std::shared_ptr<Room> r);
int Room_IsPointIn(std::shared_ptr<Room> room, btScalar dot[3]);

std::shared_ptr<Room> Room_FindPos(const btVector3 &pos);
std::shared_ptr<Room> Room_FindPosCogerrence(const btVector3& new_pos, std::shared_ptr<Room> room);
std::shared_ptr<Room> Room_GetByID(World* w, unsigned int ID);
RoomSector* Room_GetSectorRaw(std::shared_ptr<Room> room, const btVector3 &pos);
RoomSector* Room_GetSectorCheckFlip(std::shared_ptr<Room> room, btScalar pos[3]);
RoomSector* Sector_CheckFlip(RoomSector* rs);
RoomSector* Room_GetSectorXYZ(std::shared_ptr<Room> room, const btVector3 &pos);

void Room_Enable(std::shared_ptr<Room> room);
void Room_Disable(std::shared_ptr<Room> room);
void Room_SwapToAlternate(std::shared_ptr<Room> room);
void Room_SwapToBase(std::shared_ptr<Room> room);
std::shared_ptr<Room> Room_CheckFlip(std::shared_ptr<Room> r);
void Room_SwapPortals(std::shared_ptr<Room> room, std::shared_ptr<Room> dest_room); //Swap room portals of input room to destination room
void Room_SwapItems(std::shared_ptr<Room> room, std::shared_ptr<Room> dest_room);   //Swap room items of input room to destination room
void Room_BuildNearRoomsList(std::shared_ptr<Room> room);
void Room_BuildOverlappedRoomsList(std::shared_ptr<Room> room);

bool Room_IsJoined(std::shared_ptr<Room> r1, std::shared_ptr<Room> r2);
int Room_IsOverlapped(std::shared_ptr<Room> r0, std::shared_ptr<Room> r1);
int Room_IsInNearRoomsList(std::shared_ptr<Room> room, std::shared_ptr<Room> r);
int Room_HasSector(std::shared_ptr<Room> room, int x, int y);//If this room contains a sector
RoomSector* TR_Sector_CheckBaseRoom(RoomSector* rs);
RoomSector* TR_Sector_CheckAlternateRoom(RoomSector* rs);
RoomSector* TR_Sector_CheckPortalPointerRaw(RoomSector* rs);
RoomSector* TR_Sector_CheckPortalPointer(RoomSector* rs);
int Sectors_Is2SidePortals(RoomSector* s1, RoomSector* s2);

int World_AddEntity(World* world, std::shared_ptr<Entity> entity);
int World_CreateItem(World* world, uint32_t item_id, uint32_t model_id, uint32_t world_model_id, uint16_t type, uint16_t count, const char *name);
int World_DeleteItem(World* world, uint32_t item_id);
Sprite* World_GetSpriteByID(unsigned int ID, World* world);
SkeletalModel* World_GetModelByID(World* w, uint32_t id);           // binary search the model by ID


#endif
