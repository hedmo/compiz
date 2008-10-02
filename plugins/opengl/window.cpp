#include "privates.h"

GLWindow::GLWindow (CompWindow *w) :
    PrivateHandler<GLWindow, CompWindow, COMPIZ_OPENGL_ABI> (w),
    priv (new PrivateGLWindow (w, this))
{
    CompositeWindow *cw = CompositeWindow::get (w);
	
    priv->paint.opacity    = cw->opacity ();
    priv->paint.brightness = cw->brightness ();
    priv->paint.saturation = cw->saturation ();

    priv->lastPaint = priv->paint;

}

GLWindow::~GLWindow ()
{
    delete priv;
}

PrivateGLWindow::PrivateGLWindow (CompWindow *w,
				  GLWindow   *gw) :
    window (w),
    gWindow (gw),
    cWindow (CompositeWindow::get (w)),
    gScreen (GLScreen::get (screen)),
    textures (),
    regions (),
    updateReg (true),
    clip (),
    bindFailed (false),
    geometry ()
{
    paint.xScale	= 1.0f;
    paint.yScale	= 1.0f;
    paint.xTranslate	= 0.0f;
    paint.yTranslate	= 0.0f;

    WindowInterface::setHandler (w);
    CompositeWindowInterface::setHandler (cWindow);
}

PrivateGLWindow::~PrivateGLWindow ()
{
}

void
PrivateGLWindow::setWindowMatrix ()
{
    if (textures.size () != matrices.size ())
	matrices.resize (textures.size ());
    
    for (unsigned int i = 0; i < textures.size (); i++)
    {
	matrices[i] = textures[i]->matrix ();
	matrices[i].x0 -= ((window->geometry ().x () - window->input ().left) *
			   matrices[i].xx);
	matrices[i].y0 -= ((window->geometry ().y () - window->input ().top) *
			   matrices[i].yy);
    }
}

bool
GLWindow::bind ()
{
    CompWindowExtents i = priv->window->input ();

    if (!priv->cWindow->pixmap () && !priv->cWindow->bind ())
	return false;

    priv->textures =
	GLTexture::bindPixmapToTexture (priv->cWindow->pixmap (),
				        priv->window->size ().width () +
					i.left + i.right,
					priv->window->size ().height () +
					i.top + i.bottom,
					priv->window->depth ());
    if (priv->textures.empty ())
    {
	compLogMessage ("opengl", CompLogLevelInfo,
			"Couldn't bind redirected window 0x%x to "
			"texture\n", (int) priv->window->id ());
    }

    priv->setWindowMatrix ();
    priv->updateReg = true;

    return true;
}

void
GLWindow::release ()
{
    if (priv->cWindow->pixmap ())
    {
	priv->textures.clear ();
	priv->cWindow->release ();
    }
}

bool
GLWindowInterface::glPaint (const GLWindowPaintAttrib &attrib,
			    const GLMatrix            &transform,
			    const CompRegion          &region,
			    unsigned int              mask)
    WRAPABLE_DEF (glPaint, attrib, transform, region, mask)

bool
GLWindowInterface::glDraw (const GLMatrix     &transform,
			   GLFragment::Attrib &fragment,
			   const CompRegion   &region,
			   unsigned int       mask)
    WRAPABLE_DEF (glDraw, transform, fragment, region, mask)

void
GLWindowInterface::glAddGeometry (const GLTexture::MatrixList &matrix,
				  const CompRegion            &region,
				  const CompRegion            &clip)
    WRAPABLE_DEF (glAddGeometry, matrix, region, clip)

void
GLWindowInterface::glDrawTexture (GLTexture          *texture,
				  GLFragment::Attrib &fragment,
				  unsigned int       mask)
    WRAPABLE_DEF (glDrawTexture, texture, fragment, mask)

void
GLWindowInterface::glDrawGeometry ()
    WRAPABLE_DEF (glDrawGeometry)

const CompRegion &
GLWindow::clip () const
{
    return priv->clip;
}

GLWindowPaintAttrib &
GLWindow::paintAttrib ()
{
    return priv->paint;
}

GLWindowPaintAttrib &
GLWindow::lastPaintAttrib ()
{
    return priv->lastPaint;
}


void
PrivateGLWindow::resizeNotify (int dx, int dy, int dwidth, int dheight)
{
    window->resizeNotify (dx, dy, dwidth, dheight);
    setWindowMatrix ();
    updateReg = true;
    gWindow->release ();
}

void
PrivateGLWindow::moveNotify (int dx, int dy, bool now)
{
    window->moveNotify (dx, dy, now);
    updateReg = true;
    setWindowMatrix ();
}

void
PrivateGLWindow::windowNotify (CompWindowNotify n)
{
    switch (n)
    {
	case CompWindowNotifyUnmap:
	case CompWindowNotifyReparent:
	case CompWindowNotifyUnreparent:
	case CompWindowNotifyFrameUpdate:
	    gWindow->release ();
	    break;
	case CompWindowNotifyAliveChanged:
	    gWindow->updatePaintAttribs ();
	    break;
	default:
	    break;
	
    }

    window->windowNotify (n);
}

void
GLWindow::updatePaintAttribs ()
{
    CompositeWindow *cw = CompositeWindow::get (priv->window);

    if (priv->window->alive ())
    {
	priv->paint.opacity    = cw->opacity ();
	priv->paint.brightness = cw->brightness ();
	priv->paint.saturation = cw->saturation ();
    }
    else
    {
	priv->paint.opacity    = cw->opacity ();
	priv->paint.brightness = 0xa8a8;
	priv->paint.saturation = 0;
    }
}

GLWindow::Geometry &
GLWindow::geometry ()
{
    return priv->geometry;
}

GLWindow::Geometry::Geometry () :
    vertices (NULL),
    vertexSize (0),
    vertexStride (0),
    indices (NULL),
    indexSize (0),
    vCount (0),
    texUnits (0),
    texCoordSize (0),
    indexCount (0)
{
}

GLWindow::Geometry::~Geometry ()
{
    if (vertices)
	free (vertices);

    if (indices)
	free (indices);
}

void
GLWindow::Geometry::reset ()
{
    vCount = indexCount = 0;
}

bool
GLWindow::Geometry::moreVertices (int newSize)
{
    if (newSize > vertexSize)
    {
	GLfloat *nVertices;

	nVertices = (GLfloat *)
	    realloc (vertices, sizeof (GLfloat) * newSize);
	if (!nVertices)
	    return false;

	vertices = nVertices;
	vertexSize = newSize;
    }

    return true;
}

bool
GLWindow::Geometry::moreIndices (int newSize)
{
    if (newSize > indexSize)
    {
	GLushort *nIndices;

	nIndices = (GLushort *)
	    realloc (indices, sizeof (GLushort) * newSize);
	if (!nIndices)
	    return false;

	indices = nIndices;
	indexSize = newSize;
    }

    return true;
}

const GLTexture::List &
GLWindow::textures () const
{
    return priv->textures;
}

const GLTexture::MatrixList &
GLWindow::matrices () const
{
    return priv->matrices;
}

void
PrivateGLWindow::updateFrameRegion (CompRegion &region)
{
    window->updateFrameRegion (region);
    updateReg = true;
}

void
PrivateGLWindow::updateWindowRegions ()
{
    if (regions.size () != textures.size ())
	regions.resize (textures.size ());
    for (unsigned int i = 0; i < textures.size (); i++)
    {
	regions[i] = CompRegion (textures[i]->size ());
	regions[i].translate (window->geometry ().x () - window->input ().left,
			      window->geometry ().y () - window->input ().top);
	regions[i].handle ();
	regions[i] &= window->region ();
	regions[i].handle ();
    }
    updateReg = false;
}
