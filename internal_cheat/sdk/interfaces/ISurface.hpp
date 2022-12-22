#pragma once

#include "IAppSystem.hpp"
#include "../math/Vector2D.hpp"
#include "../misc/Color.hpp"

namespace vgui {
	typedef unsigned long HFont;
	typedef unsigned int VPANEL;
};

enum FontFeature {
	FONT_FEATURE_ANTIALIASED_FONTS = 1,
	FONT_FEATURE_DROPSHADOW_FONTS = 2,
	FONT_FEATURE_OUTLINE_FONTS = 6,
};

enum FontDrawType {
	FONT_DRAW_DEFAULT = 0,
	FONT_DRAW_NONADDITIVE,
	FONT_DRAW_ADDITIVE,
	FONT_DRAW_TYPE_COUNT = 2,
};

enum FontFlags {
	FONTFLAG_NONE,
	FONTFLAG_ITALIC = 0x001,
	FONTFLAG_UNDERLINE = 0x002,
	FONTFLAG_STRIKEOUT = 0x004,
	FONTFLAG_SYMBOL = 0x008,
	FONTFLAG_ANTIALIAS = 0x010,
	FONTFLAG_GAUSSIANBLUR = 0x020,
	FONTFLAG_ROTARY = 0x040,
	FONTFLAG_DROPSHADOW = 0x080,
	FONTFLAG_ADDITIVE = 0x100,
	FONTFLAG_OUTLINE = 0x200,
	FONTFLAG_CUSTOM = 0x400,
	FONTFLAG_BITMAP = 0x800,
};

struct IntRect {
	int x0;
	int y0;
	int x1;
	int y1;
};

struct Vertex_t {
	Vertex_t() { }
	Vertex_t(const Vector2D &pos, const Vector2D &coord = Vector2D(0, 0)) { //-V818
		m_Position = pos;
		m_TexCoord = coord;
	}
	void Init(const Vector2D &pos, const Vector2D &coord = Vector2D(0, 0)) {
		m_Position = pos;
		m_TexCoord = coord;
	}

	Vector2D m_Position;
	Vector2D m_TexCoord;
};

//-----------------------------------------------------------------------------
// Purpose: Wraps contextless windows system functions
//-----------------------------------------------------------------------------
class ISurface : public IAppSystem {
public:
	virtual void          RunFrame() = 0; // 9
	virtual vgui::VPANEL  GetEmbeddedPanel() = 0; // 10
	virtual void          SetEmbeddedPanel(vgui::VPANEL pPanel) = 0; // 11
	virtual void          PushMakeCurrent(vgui::VPANEL panel, bool useInsets) = 0; // 12
	virtual void          PopMakeCurrent(vgui::VPANEL panel) = 0; // 13
	virtual void          DrawSetColor(int r, int g, int b, int a) = 0; // 14
	virtual void          DrawSetColor(Color col) = 0; // 15
	virtual void          DrawFilledRect(int x0, int y0, int x1, int y1) = 0; // 16
	virtual void          DrawFilledRectArray(IntRect *pRects, int numRects) = 0; // 17
	virtual void          DrawOutlinedRect(int x0, int y0, int x1, int y1) = 0; // 18
	virtual void          DrawLine(int x0, int y0, int x1, int y1) = 0; // 19
	virtual void          DrawPolyLine(int *px, int *py, int numPoints) = 0; // 20
	virtual void          DrawSetApparentDepth(float f) = 0; // 21
	virtual void          DrawClearApparentDepth(void) = 0; // 22
	virtual void          DrawSetTextFont(vgui::HFont font) = 0; // 23
	virtual void          DrawSetTextColor(int r, int g, int b, int a) = 0; // 24
	virtual void          DrawSetTextColor(Color col) = 0; // 25
	virtual void          DrawSetTextPos(int x, int y) = 0; // 26
	virtual void          DrawGetTextPos(int& x, int& y) = 0; // 27
	virtual void          DrawPrintText(const wchar_t *text, int textLen, FontDrawType drawType = FontDrawType::FONT_DRAW_DEFAULT) = 0; // 28
	virtual void          DrawUnicodeChar(wchar_t wch, FontDrawType drawType = FontDrawType::FONT_DRAW_DEFAULT) = 0; // 29
	virtual void          DrawFlushText() = 0; // 30
	virtual void*         CreateHTMLWindow(void *events, vgui::VPANEL context) = 0; // 31
	virtual void          PaintHTMLWindow(void *htmlwin) = 0; // 32
	virtual void          DeleteHTMLWindow(void *htmlwin) = 0; // 33
	virtual int           DrawGetTextureId(char const *filename) = 0; // 34
	virtual bool          DrawGetTextureFile(int id, char *filename, int maxlen) = 0; // 35
	virtual void          DrawSetTextureFile(int id, const char *filename, int hardwareFilter, bool forceReload) = 0; // 36
	virtual void          DrawSetTextureRGBA(int id, const unsigned char *rgba, int wide, int tall) = 0; // 37
	virtual void          DrawSetTexture(int id) = 0; // 38
	virtual void          DeleteTextureByID(int id) = 0; // 39
	virtual void          DrawGetTextureSize(int id, int &wide, int &tall) = 0; // 40
	virtual void          DrawTexturedRect(int x0, int y0, int x1, int y1) = 0; // 41
	virtual bool          IsTextureIDValid(int id) = 0; // 42
	virtual int           CreateNewTextureID(bool procedural = false) = 0; // 43
	virtual void          GetScreenSize(int &wide, int &tall) = 0; // 44
	virtual void          SetAsTopMost(vgui::VPANEL panel, bool state) = 0; // 45
	virtual void          BringToFront(vgui::VPANEL panel) = 0; // 46
	virtual void          SetForegroundWindow(vgui::VPANEL panel) = 0; // 47
	virtual void          SetPanelVisible(vgui::VPANEL panel, bool state) = 0; // 48 
	virtual void          SetMinimized(vgui::VPANEL panel, bool state) = 0; // 49
	virtual bool          IsMinimized(vgui::VPANEL panel) = 0; // 50
	virtual void          FlashWindow(vgui::VPANEL panel, bool state) = 0; // 51
	virtual void          SetTitle(vgui::VPANEL panel, const wchar_t *title) = 0; // 52
	virtual void          SetAsToolBar(vgui::VPANEL panel, bool state) = 0; // 53
	virtual void          CreatePopup(vgui::VPANEL panel, bool minimised, bool showTaskbarIcon = true, bool disabled = false, bool mouseInput = true, bool kbInput = true) = 0; // 54
	virtual void          SwapBuffers(vgui::VPANEL panel) = 0; // 55
	virtual void          Invalidate(vgui::VPANEL panel) = 0; // 56
	virtual void          SetCursor(unsigned long cursor) = 0; // 57
	virtual bool          IsCursorVisible() = 0; // 58
	virtual void          ApplyChanges() = 0; // 59
	virtual bool          IsWithin(int x, int y) = 0; // 60
	virtual bool          HasFocus() = 0; // 61
	virtual bool          SupportsFeature(int /*SurfaceFeature_t*/ feature) = 0; // 62
	virtual void          RestrictPaintToSinglePanel(vgui::VPANEL panel, bool bForceAllowNonModalSurface = false) = 0; // 63
	virtual void          SetModalPanel(vgui::VPANEL) = 0; // 64
	virtual vgui::VPANEL  GetModalPanel() = 0; // 65
	virtual void          UnlockCursor() = 0; // 66
	virtual void          LockCursor() = 0; // 67 
	virtual void          SetTranslateExtendedKeys(bool state) = 0; // 68
	virtual vgui::VPANEL  GetTopmostPopup() = 0; // 69
	virtual void          SetTopLevelFocus(vgui::VPANEL panel) = 0; // 70
	virtual vgui::HFont   FontCreate() = 0; // 71
	virtual bool          SetFontGlyphSet(vgui::HFont font, const char *windowsFontName, int tall, int weight, int blur, int scanlines, int flags, int nRangeMin = 0, int nRangeMax = 0) = 0; // 72
	virtual bool          AddCustomFontFile(const char *fontFileName) = 0; // 73
	virtual int           GetFontTall(vgui::HFont font) = 0; // 74
	virtual int           GetFontAscent(vgui::HFont font, wchar_t wch) = 0; // 75
	virtual bool          IsFontAdditive(vgui::HFont font) = 0; // 76
	virtual void          GetCharABCwide(vgui::HFont font, int ch, int &a, int &b, int &c) = 0; // 77
	virtual int           GetCharacterWidth(vgui::HFont font, int ch) = 0; // 78
	virtual void          GetTextSize(vgui::HFont font, const wchar_t *text, int &wide, int &tall) = 0; // 79
	virtual vgui::VPANEL  GetNotifyPanel() = 0; // 80
	virtual void          SetNotifyIcon(vgui::VPANEL context, unsigned long icon, vgui::VPANEL panelToReceiveMessages, const char *text) = 0; // 81
	virtual void          PlaySound_(const char *fileName) = 0; // 82
	virtual int           GetPopupCount() = 0; // 83
	virtual vgui::VPANEL  GetPopup(int index) = 0; // 84
	virtual bool          ShouldPaintChildPanel(vgui::VPANEL childPanel) = 0; // 85
	virtual bool          RecreateContext(vgui::VPANEL panel) = 0; // 86
	virtual void          AddPanel(vgui::VPANEL panel) = 0; // 87
	virtual void          ReleasePanel(vgui::VPANEL panel) = 0; // 88
	virtual void          MovePopupToFront(vgui::VPANEL panel) = 0; // 89
	virtual void          MovePopupToBack(vgui::VPANEL panel) = 0; // 90
	virtual void          SolveTraverse(vgui::VPANEL panel, bool forceApplySchemeSettings = false) = 0; // 91
	virtual void          PaintTraverse(vgui::VPANEL panel) = 0; // 92
	virtual void          EnableMouseCapture(vgui::VPANEL panel, bool state) = 0; // 93
	virtual void          GetWorkspaceBounds(int &x, int &y, int &wide, int &tall) = 0; // 94
	virtual void          GetAbsoluteWindowBounds(int &x, int &y, int &wide, int &tall) = 0; // 95
	virtual void          GetProportionalBase(int &width, int &height) = 0; // 96
	virtual void          CalculateMouseVisible() = 0; // 97
	virtual bool          NeedKBInput() = 0; // 98
	virtual bool          HasCursorPosFunctions() = 0; // 99
	virtual void          SurfaceGetCursorPos(int &x, int &y) = 0; // 100
	virtual void          SurfaceSetCursorPos(int x, int y) = 0; // 101
	virtual void          DrawTexturedLine(const Vertex_t &a, const Vertex_t &b) = 0; // 102
	virtual void          DrawOutlinedCircle(int x, int y, int radius, int segments) = 0; // 103
	virtual void          DrawTexturedPolyLine(const Vertex_t *p, int n) = 0; // 104
	virtual void          DrawTexturedSubRect(int x0, int y0, int x1, int y1, float texs0, float text0, float texs1, float text1) = 0; // 105
	virtual void          DrawTexturedPolygon(int n, Vertex_t *pVertice, bool bClipVertices = true) = 0; // 106
};