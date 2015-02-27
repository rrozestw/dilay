#ifndef DILAY_VIEW_GL_WIDGET
#define DILAY_VIEW_GL_WIDGET

#include <QOpenGLWidget>
#include <glm/fwd.hpp>
#include "macro.hpp"

class Cache;
class Config;
class ViewMainWindow;

class ViewGlWidget : public QOpenGLWidget {
    Q_OBJECT
  public:
    DECLARE_BIG2 (ViewGlWidget, ViewMainWindow&, Config&, Cache&)

    glm::ivec2 cursorPosition       ();
    void       selectIntersection   ();
    void       selectIntersection   (const glm::ivec2&);
       
  protected:
    void initializeGL       ();
    void resizeGL           (int,int);
    void paintGL            ();
 
    void keyPressEvent      (QKeyEvent*);
    void mouseMoveEvent     (QMouseEvent*);
    void mousePressEvent    (QMouseEvent*);
    void mouseReleaseEvent  (QMouseEvent*);
    void wheelEvent         (QWheelEvent*);

  private:
    IMPLEMENTATION
};

#endif
