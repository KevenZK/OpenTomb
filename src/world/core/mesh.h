#pragma once

#include <array>
#include <memory>
#include <vector>

#include <LinearMath/btTransform.h>
#include <LinearMath/btScalar.h>
#include <LinearMath/btVector3.h>

#include "LuaState.h"

#include "world/object.h"
#include "render/vertex_array.h"
#include "loader/datatypes.h"
#include "obb.h"

struct Character;
class btCollisionShape;
class btRigidBody;
class btCollisionShape;

namespace engine
{
struct EngineContainer;
} // namespace engine

namespace render
{
class Render;
} // namespace render

namespace world
{
struct RoomSector;
struct SectorTween;
struct Room;
struct Entity;
enum class AnimUpdate;

namespace core
{

#define ANIM_CMD_MOVE               0x01
#define ANIM_CMD_CHANGE_DIRECTION   0x02
#define ANIM_CMD_JUMP               0x04


struct Polygon;
struct Vertex;

struct TransparentPolygonReference
{
    const Polygon *polygon;
    std::shared_ptr< ::render::VertexArray > used_vertex_array;
    size_t firstIndex;
    size_t count;
    bool isAnimated;
};

/*
 * Animated version of vertex. Does not contain texture coordinate, because that is in a different VBO.
 */
struct AnimatedVertex
{
    btVector3 position;
    std::array<float, 4> color;
    btVector3 normal;
};

/*
 * base mesh, uses everywhere
 */
struct BaseMesh
{
    uint32_t m_id;                                                   // mesh's ID
    bool m_usesVertexColors;                                   // does this mesh have prebaked vertex lighting

    std::vector<Polygon> m_polygons;                                             // polygons data

    std::vector<Polygon> m_transparencyPolygons;                                // transparency mesh's polygons list

    uint32_t              m_texturePageCount;                                    // face without structure wrapping
    std::vector<uint32_t> m_elementsPerTexture;                            //
    std::vector<GLuint> m_elements;                                             //
    uint32_t m_alphaElements;

    std::vector<Vertex> m_vertices;

    size_t m_animatedElementCount;
    size_t m_alphaAnimatedElementCount;
    std::vector<GLuint> m_allAnimatedElements;
    std::vector<AnimatedVertex> m_animatedVertices;

    std::vector<TransparentPolygonReference> m_transparentPolygons;

    btVector3 m_center;                                            // geometry centre of mesh
    btVector3 m_bbMin;                                            // AABB bounding volume
    btVector3 m_bbMax;                                            // AABB bounding volume
    btScalar m_radius;                                            // radius of the bounding sphere
#pragma pack(push,1)
    struct MatrixIndex
    {
        int8_t i = 0, j = 0;
    };
#pragma pack(pop)
    std::vector<MatrixIndex> m_matrixIndices;                                       // vertices map for skin mesh

    GLuint                m_vboVertexArray = 0;
    GLuint                m_vboIndexArray = 0;
    GLuint                m_vboSkinArray = 0;
    std::shared_ptr< ::render::VertexArray > m_mainVertexArray;

    // Buffers for animated polygons
    // The first contains position, normal and color.
    // The second contains the texture coordinates. It gets updated every frame.
    GLuint                m_animatedVboVertexArray;
    GLuint                m_animatedVboTexCoordArray;
    GLuint                m_animatedVboIndexArray;
    std::shared_ptr< ::render::VertexArray > m_animatedVertexArray;

    ~BaseMesh()
    {
        clear();
    }

    void clear();
    void updateBoundingBox();
    void genVBO(const render::Render *renderer);
    void genFaces();
    uint32_t addVertex(const Vertex& v);
    uint32_t addAnimatedVertex(const Vertex& v);
    void polySortInMesh();
    Vertex* findVertex(const btVector3& v);
};

/*
 * base sprite structure
 */
struct Sprite
{
    uint32_t            id;                                                     // object's ID
    uint32_t            texture;                                                // texture number
    GLfloat             tex_coord[8];                                           // texture coordinates
    uint32_t            flag;
    btScalar            left;                                                   // world sprite's gabarites
    btScalar            right;
    btScalar            top;
    btScalar            bottom;
};

/*
 * Structure for all the sprites in a room
 */
struct SpriteBuffer
{
    // Vertex data for the sprites
    std::unique_ptr< ::render::VertexArray > data{};

    // How many sub-ranges the element_array_buffer contains. It has one for each texture listed.
    uint32_t              num_texture_pages = 0;
    // The element count for each sub-range.
    std::vector<uint32_t> element_count_per_texture{};
};

struct Light
{
    btVector3 pos;                                         // world position
    float                       colour[4];                                      // RGBA value

    float                       inner;
    float                       outer;
    float                       length;
    float                       cutoff;

    float                       falloff;

    loader::LightType           light_type;
};

/*
 *  Animated sequence. Used globally with animated textures to refer its parameters and frame numbers.
 */

struct TexFrame
{
    btScalar    mat[4];
    btScalar    move[2];
    uint16_t    tex_ind;
};

// Animated texture types
enum class AnimTextureType
{
    Forward,
    Backward,
    Reverse
};

struct AnimSeq
{
    bool        uvrotate;               // UVRotate mode flag.
    bool        frame_lock;             // Single frame mode. Needed for TR4-5 compatible UVRotate.

    bool        blend;                  // Blend flag.  Reserved for future use!
    btScalar    blend_rate;             // Blend rate.  Reserved for future use!
    btScalar    blend_time;             // Blend value. Reserved for future use!

    AnimTextureType anim_type;          // 0 = normal, 1 = back, 2 = reverse.
    bool        reverse_direction;      // Used only with type 2 to identify current animation direction.
    btScalar    frame_time;             // Time passed since last frame update.
    uint16_t    current_frame;          // Current frame for this sequence.
    btScalar    frame_rate;             // For types 0-1, specifies framerate, for type 3, should specify rotation speed.

    btScalar    uvrotate_speed;         // Speed of UVRotation, in seconds.
    btScalar    uvrotate_max;           // Reference value used to restart rotation.
    btScalar    current_uvrotate;       // Current coordinate window position.

    std::vector<TexFrame> frames;
    std::vector<uint32_t> frame_list;   // Offset into anim textures frame list.
};

/*
 * room static mesh.
 */
struct StaticMesh : public Object
{
    uint32_t                    object_id;                                      //
    uint8_t                     was_rendered;                                   // 0 - was not rendered, 1 - opaque, 2 - transparency, 3 - full rendered
    uint8_t                     was_rendered_lines;
    bool hide;                                           // disable static mesh rendering
    btVector3 pos;                                         // model position
    btVector3 rot;                                         // model angles
    std::array<float, 4> tint;                                        // model tint

    btVector3 vbb_min;                                     // visible bounding box
    btVector3 vbb_max;
    btVector3 cbb_min;                                     // collision bounding box
    btVector3 cbb_max;

    btTransform transform;                                  // gl transformation matrix
    OrientedBoundingBox obb;
    std::shared_ptr<engine::EngineContainer> self;

    std::shared_ptr<BaseMesh> mesh;                                           // base model
    btRigidBody                *bt_body;
};

/*
 * Animated skeletal model. Taken from openraider.
 * model -> animation -> frame -> bone
 * thanks to Terry 'Mongoose' Hendrix II
 */

 /*
  * SMOOTHED ANIMATIONS STRUCTURES
  * stack matrices are needed for skinned mesh transformations.
  */
struct SSBoneTag
{
    SSBoneTag   *parent;
    uint16_t                index;
    std::shared_ptr<BaseMesh> mesh_base;                                          // base mesh - pointer to the first mesh in array
    std::shared_ptr<BaseMesh> mesh_skin;                                          // base skinned mesh for ТР4+
    std::shared_ptr<BaseMesh> mesh_slot;
    btVector3 offset;                                          // model position offset

    btQuaternion qrotate;                                         // quaternion rotation
    btTransform transform;    // 4x4 OpenGL matrix for stack usage
    btTransform full_transform;    // 4x4 OpenGL matrix for global usage

    uint32_t                body_part;                                          // flag: BODY, LEFT_LEG_1, RIGHT_HAND_2, HEAD...
};

struct SkeletalModel;

struct SSAnimation
{
    int16_t                     last_state = 0;
    int16_t                     next_state = 0;
    int16_t                     last_animation = 0;
    int16_t                     current_animation = 0;                              //
    int16_t                     next_animation = 0;                                 //
    //! @todo Many comparisons with unsigned, so check if it can be made unsigned.
    int16_t                     current_frame = 0;                                  //
    int16_t                     next_frame = 0;                                     //

    uint16_t                    anim_flags = 0;                                     // additional animation control param

    btScalar                    period = 1.0f / 30;                                 // one frame change period
    btScalar                    frame_time = 0;                                     // current time
    btScalar                    lerp = 0;

    void(*onFrame)(Character* ent, SSAnimation *ss_anim, AnimUpdate state);

    SkeletalModel    *model = nullptr;                                          // pointer to the base model
    SSAnimation      *next = nullptr;
};

/*
 * base frame of animated skeletal model
 */
struct SSBoneFrame
{
    std::vector<SSBoneTag> bone_tags;                                      // array of bones
    btVector3 pos;                                         // position (base offset)
    btVector3 bb_min;                                      // bounding box min coordinates
    btVector3 bb_max;                                      // bounding box max coordinates
    btVector3 centre;                                      // bounding box centre

    SSAnimation       animations;                                     // animations list

    bool hasSkin;                                       // whether any skinned meshes need rendering

    void fromModel(SkeletalModel* model);
};

/*
 * ORIGINAL ANIMATIONS
 */
struct BoneTag
{
    btVector3 offset;                                            // bone vector
    btQuaternion qrotate;                                           // rotation quaternion
};

/*
 * base frame of animated skeletal model
 */
struct BoneFrame
{
    uint16_t            command;                                                // & 0x01 - move need, &0x02 - 180 rotate need
    std::vector<BoneTag> bone_tags;                                              // bones data
    btVector3 pos;                                                 // position (base offset)
    btVector3 bb_min;                                              // bounding box min coordinates
    btVector3 bb_max;                                              // bounding box max coordinates
    btVector3 centre;                                              // bounding box centre
    btVector3 move;                                                // move command data
    btScalar            v_Vertical;                                             // jump command data
    btScalar            v_Horizontal;                                           // jump command data
};

/*
 * mesh tree base element structure
 */
struct MeshTreeTag
{
    std::shared_ptr<BaseMesh> mesh_base;                                      // base mesh - pointer to the first mesh in array
    std::shared_ptr<BaseMesh> mesh_skin;                                      // base skinned mesh for ТР4+
    btVector3 offset;                                      // model position offset
    uint16_t                    flag;                                           // 0x0001 = POP, 0x0002 = PUSH, 0x0003 = RESET
    uint32_t                    body_part;
    uint8_t                     replace_mesh;                                   // flag for shoot / guns animations (0x00, 0x01, 0x02, 0x03)
    uint8_t                     replace_anim;
};

/*
 * animation switching control structure
 */
struct AnimDispatch
{
    uint16_t    next_anim;                                                      // "switch to" animation
    uint16_t    next_frame;                                                     // "switch to" frame
    uint16_t    frame_low;                                                      // low border of state change condition
    uint16_t    frame_high;                                                     // high border of state change condition
};

struct StateChange
{
    uint32_t                    id;
    std::vector<AnimDispatch> anim_dispatch;
};

/*
 * one animation frame structure
 */
struct AnimationFrame
{
    uint32_t                    id;
    uint8_t                     original_frame_rate;
    int32_t                     speed_x;                // Forward-backward speed
    int32_t                     accel_x;                // Forward-backward accel
    int32_t                     speed_y;                // Left-right speed
    int32_t                     accel_y;                // Left-right accel
    uint32_t                    anim_command;
    uint32_t                    num_anim_commands;
    uint16_t                    state_id;
    std::vector<BoneFrame> frames;                 // Frame data

    std::vector<StateChange> stateChanges;           // Animation statechanges data

    AnimationFrame   *next_anim;              // Next default animation
    int                         next_frame;             // Next default frame

    StateChange* findStateChangeByAnim(int state_change_anim)
    {
        if(state_change_anim < 0)
            return nullptr;

        for(StateChange& stateChange : stateChanges)
        {
            for(const AnimDispatch& dispatch : stateChange.anim_dispatch)
            {
                if(dispatch.next_anim == state_change_anim)
                {
                    return &stateChange;
                }
            }
        }

        return nullptr;
    }

    StateChange* findStateChangeByID(uint32_t id)
    {
        for(StateChange& stateChange : stateChanges)
        {
            if(stateChange.id == id)
            {
                return &stateChange;
            }
        }

        return nullptr;
    }
};

/*
 * skeletal model with animations data.
 */

struct SkeletalModel
{
    uint32_t                    id;
    bool                        has_transparency;

    btVector3                   bbox_min;
    btVector3                   bbox_max;
    btVector3                   centre;                                      // the centre of model

    std::vector<AnimationFrame> animations;

    uint16_t                    mesh_count;                                     // number of model meshes
    std::vector<MeshTreeTag>    mesh_tree;                                      // base mesh tree.

    std::vector<uint16_t>       collision_map;

    void clear();
    void updateTransparencyFlag();
    void interpolateFrames();
    void fillSkinnedMeshMap();
};


void BoneFrame_Copy(BoneFrame* dst, BoneFrame* src);
MeshTreeTag* SkeletonClone(MeshTreeTag* src, int tags_count);
void SkeletonCopyMeshes(MeshTreeTag* dst, MeshTreeTag* src, int tags_count);
void SkeletonCopyMeshes2(MeshTreeTag* dst, MeshTreeTag* src, int tags_count);

/* bullet collision model calculation */
btCollisionShape *BT_CSfromSphere(const btScalar& radius);
btCollisionShape* BT_CSfromBBox(const btVector3 &bb_min, const btVector3 &bb_max, bool useCompression, bool buildBvh);
btCollisionShape* BT_CSfromMesh(const std::shared_ptr<BaseMesh> &mesh, bool useCompression, bool buildBvh, bool is_static = true);
btCollisionShape* BT_CSfromHeightmap(const std::vector<RoomSector> &heightmap, const std::vector<SectorTween> &tweens, bool useCompression, bool buildBvh);

} // namespace core
} // namespace world
