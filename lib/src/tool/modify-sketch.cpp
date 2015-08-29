/* This file is part of Dilay
 * Copyright © 2015 Alexander Bau
 * Use and redistribute under the terms of the GNU General Public License
 */
#include <QCheckBox>
#include <QFrame>
#include <QMouseEvent>
#include <QPushButton>
#include "cache.hpp"
#include "scene.hpp"
#include "sketch/bone-intersection.hpp"
#include "sketch/mesh.hpp"
#include "sketch/node-intersection.hpp"
#include "state.hpp"
#include "tool/util/movement.hpp"
#include "tool/util/scaling.hpp"
#include "tools.hpp"
#include "view/properties.hpp"
#include "view/tool-tip.hpp"
#include "view/util.hpp"

struct ToolModifySketch::Impl {
  ToolModifySketch* self;
  SketchMesh*       mesh;
  SketchNode*       node;
  ToolUtilMovement  movement;
  ToolUtilScaling   scaling;
  bool              transformChildren;

  Impl (ToolModifySketch* s)
    : self              (s)
    , mesh              (nullptr)
    , node              (nullptr)
    , movement          ( s->state ().camera ()
                        , s->cache ().getFrom <MovementConstraint>
                            ("constraint", MovementConstraint::CameraPlane) )
    , scaling           (s->state ().camera ())
    , transformChildren (s->cache ().get <bool> ("transform-children", false))
  {
    this->self->renderMirror (false);

    this->setupProperties ();
    this->setupToolTip    ();
  }

  void mirrorSketchMeshes () {
    assert (this->self->hasMirror ());

    this->self->snapshotSketchMeshes ();

    this->self->state ().scene ().forEachMesh (
      [this] (SketchMesh& mesh) {
        mesh.mirror (*this->self->mirrorDimension ());
      }
    );
  }

  void setupProperties () {
    ViewPropertiesPart& properties = this->self->properties ().body ();

    properties.add (QObject::tr ("Move along"), ViewUtil::emptyWidget ());
    this->movement.addProperties (properties, [this] () {
      this->self->cache ().set ("constraint", this->movement.constraint ());
    });
    properties.add (ViewUtil::horizontalLine ());

    QPushButton& syncButton = ViewUtil::pushButton (QObject::tr ("Sync"));
    ViewUtil::connect (syncButton, [this] () {
      this->mirrorSketchMeshes ();
      this->self->updateGlWidget ();
    });
    syncButton.setEnabled (this->self->hasMirror ());

    QCheckBox& mirrorEdit = ViewUtil::checkBox ( QObject::tr ("Mirror")
                                               , this->self->hasMirror () );
    ViewUtil::connect (mirrorEdit, [this, &syncButton] (bool m) {
      this->self->mirror (m);
      syncButton.setEnabled (m);
    });
    properties.add (mirrorEdit, syncButton);

    QCheckBox& transformCEdit = ViewUtil::checkBox ( QObject::tr ("Transform children")
                                                   , this->transformChildren );
    ViewUtil::connect (transformCEdit, [this] (bool m) {
      this->transformChildren = m;
      this->self->cache ().set ("transform-children", m);
    });
    properties.add (transformCEdit);
  }

  void setupToolTip () {
    ViewToolTip toolTip;
    toolTip.add ( ViewToolTip::MouseEvent::Left, QObject::tr ("Drag to move"));
    toolTip.add ( ViewToolTip::MouseEvent::Left, ViewToolTip::Modifier::Shift
                , QObject::tr ("Drag to scale") );
    toolTip.add ( ViewToolTip::MouseEvent::Left, ViewToolTip::Modifier::Ctrl
                , QObject::tr ("Drag to add new node") );
    this->self->showToolTip (toolTip);
  }

  ToolResponse runMouseMoveEvent (const QMouseEvent& e) {
    if (e.buttons () == Qt::LeftButton && this->node) {
      if (e.modifiers () == Qt::ShiftModifier) {
        if (this->scaling.move (e)) {
          this->mesh->scale ( *this->node, this->scaling.factor ()
                            , this->transformChildren, this->self->mirrorDimension () );
        }
      }
      else if (this->movement.move (e, false)) {
        this->mesh->move (*this->node, this->movement.delta ()
                         , this->transformChildren, this->self->mirrorDimension () );
      }
      return ToolResponse::Redraw;
    }
    else {
      return ToolResponse::None;
    }
  }

  ToolResponse runMousePressEvent (const QMouseEvent& e) {

    auto handleNodeIntersection = [this, &e] (SketchNodeIntersection &intersection) {
      this->self->snapshotSketchMeshes ();

      this->movement.resetPosition ( intersection.position ());
      this->scaling .resetPosition ( intersection.node ().data ().position ()
                                   , intersection.position () );

      this->mesh = &intersection.mesh ();

      if (e.modifiers () == Qt::ControlModifier) {
        SketchNode& iNode = intersection.node ();

        const float radius = iNode.numChildren () > 0
                           ? iNode.lastChild ().data ().radius ()
                           : iNode.data ().radius ();

        this->node = &this->mesh->addChild ( iNode, iNode.data ().position ()
                                           , radius, this->self->mirrorDimension () );
      }
      else {
        this->node = &intersection.node ();
      }
    };

    auto handleBoneIntersection = [this, &e] (SketchBoneIntersection &intersection) {
      this->self->snapshotSketchMeshes ();

      this->movement.resetPosition ( intersection.position ());
      this->scaling .resetPosition ( intersection.projectedPosition ()
                                   , intersection.position () );

      this->mesh = &intersection.mesh ();

      if (e.modifiers () == Qt::ControlModifier) {
        const float radius = glm::distance ( intersection.projectedPosition ()
                                           , intersection.position () );

        this->node = &this->mesh->addParent ( intersection.child ()
                                            , intersection.projectedPosition ()
                                            , radius, this->self->mirrorDimension () );
      }
    };

    if (e.button () == Qt::LeftButton) {
      SketchNodeIntersection nodeIntersection;
      SketchBoneIntersection boneIntersection;

      if (this->self->intersectsScene (e, nodeIntersection)) {
        handleNodeIntersection (nodeIntersection);
      }
      else if (this->self->intersectsScene (e, boneIntersection)) {
        handleBoneIntersection (boneIntersection);
      }
    }
    return ToolResponse::None;
  }

  ToolResponse runMouseReleaseEvent (const QMouseEvent& e) {
    if (e.button () == Qt::LeftButton) {
      this->mesh = nullptr;
      this->node = nullptr;
    }
    return ToolResponse::None;
  }
};

DELEGATE_TOOL                         (ToolModifySketch)
DELEGATE_TOOL_RUN_MOUSE_MOVE_EVENT    (ToolModifySketch)
DELEGATE_TOOL_RUN_MOUSE_PRESS_EVENT   (ToolModifySketch)
DELEGATE_TOOL_RUN_MOUSE_RELEASE_EVENT (ToolModifySketch)