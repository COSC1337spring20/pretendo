
#include "QtVideo.h"
#include <QGLShader>
#include <algorithm>
#include <cassert>
#include <iostream>
#ifdef _WIN32
#include "glext.h"
#endif

namespace {

const int Width  = 256;
const int Height = 240;

}

//------------------------------------------------------------------------------
// Name: QtVideo
//------------------------------------------------------------------------------
QtVideo::QtVideo(QWidget *parent, const QGLWidget *shareWidget, Qt::WindowFlags f) : QGLWidget(parent, shareWidget, f), buffer_(0), texture_(0) {

	buffer_ = new uint32_t[Width * Height]();
	for(int i = 0; i < Height; ++i) {
		scanlines_[i] = &buffer_[i * Width];
	}

	setAutoBufferSwap(true);
	setFormat(QGLFormat(QGL::DoubleBuffer));
	setMouseTracking(false);
	setBaseSize(Width, Height);

	connect(this, SIGNAL(render_frame()), this, SLOT(updateGL()));

	std::cout << "[QtVideo::QtVideo]" << std::endl;
}

//------------------------------------------------------------------------------
// Name: ~QtVideo
//------------------------------------------------------------------------------
QtVideo::~QtVideo() {
	delete [] buffer_;
}

//------------------------------------------------------------------------------
// Name: resizeGL
//------------------------------------------------------------------------------
void QtVideo::resizeGL(int width, int height) {
	Q_UNUSED(width);
	Q_UNUSED(height);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
void QtVideo::initializeGL() {

	glDisable(GL_ALPHA_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_POLYGON_SMOOTH);
	glDisable(GL_STENCIL_TEST);
	glEnable(GL_DITHER);
	glEnable(GL_TEXTURE_2D);
	glClearColor(0.0, 0.0, 0.0, 0.0);

	glGenTextures(1, &texture_);
	glBindTexture(GL_TEXTURE_2D, texture_);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, Width);
	
	// clamp out of bounds texture coordinates
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	
	// link the texture with the buffer
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Width, Height, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, buffer_);
	

	
#if 0
	QGLShaderProgram program(context());
	
	program.addShaderFromSourceCode(QGLShader::Vertex,
	"    void main(void) {\n"
	"      gl_Position = ftransform();\n"
	"      gl_TexCoord[0] = gl_MultiTexCoord0;\n"
	"    }"
	);
	
	program.addShaderFromSourceCode(QGLShader::Fragment,
    "    uniform sampler2D rubyTexture;\n"
	"    void main(void) {   \n"
	"      vec4 rgb = texture2D(rubyTexture, gl_TexCoord[0].xy);\n"
	"      vec4 intens = smoothstep(0.2, 0.8,rgb) + normalize(vec4(rgb.xyz, 1.0));\n"
	"      if(fract(gl_FragCoord.y * 0.5) > 0.5) intens = rgb * 0.8;\n"
	"      gl_FragColor = intens;\n"
	"    }"
	);
	
	program.link();
	program.bind();
#endif
	

}

//------------------------------------------------------------------------------
// Name: submit_scanline
//------------------------------------------------------------------------------
void QtVideo::submit_scanline(int scanline, int intensity, const uint8_t *source) {
	
	const uint32_t *const palette = palette_[intensity];
	
#if 1
	uint64_t *s = reinterpret_cast<uint64_t *>(scanlines_[scanline]);
	for(int i = 0; i < Width; ) {
	
		// this approach will try to read 32-bits from the source at a time...
		const uint32_t pixels = *reinterpret_cast<const uint32_t *>(source + i);
		const uint8_t pix0 = (pixels >> 0x00) & 0xff;
		const uint8_t pix1 = (pixels >> 0x08) & 0xff;
		const uint8_t pix2 = (pixels >> 0x10) & 0xff;
		const uint8_t pix3 = (pixels >> 0x18) & 0xff;
	
		// collect them into 128-bits of data (unfonately with some indirection)
		uint64_t value0 = 
			(static_cast<uint64_t>(palette[pix0]) << 0) |
			(static_cast<uint64_t>(palette[pix1]) << 32);

		uint64_t value1 = 
			(static_cast<uint64_t>(palette[pix2]) << 0) |
			(static_cast<uint64_t>(palette[pix3]) << 32);
	
	
		// then write them, this will help the rendering be much more
		// cache friendly
		*s++ = value0;
		*s++ = value1;
		
		i += 4;
	}
#else
	uint32_t *const s = scanlines_[scanline];
	std::transform(source, source + Width, s, [palette](uint8_t index) {
		return palette[index];
	});
#endif
}

//------------------------------------------------------------------------------
// Name: set_palette
//------------------------------------------------------------------------------
void QtVideo::set_palette(const color_emphasis_t *intensity, const rgb_color_t *pal) {
	assert(pal);
	assert(intensity);
	
	std::cout << "Setting Palette" << std::endl;

	for(int j = 0; j < 8; j++) {
		for(int i = 0; i < 64; i++) {
			palette_[j][i] =
				QColor(
					qBound(0x00, static_cast<int>(pal[i].r * intensity[j].r), 0xff),
					qBound(0x00, static_cast<int>(pal[i].g * intensity[j].g), 0xff),
					qBound(0x00, static_cast<int>(pal[i].b * intensity[j].b), 0xff)).rgb();

		}
	}
}

//------------------------------------------------------------------------------
// Name: start_frame
//------------------------------------------------------------------------------
void QtVideo::start_frame() {
}

//------------------------------------------------------------------------------
// Name: end_frame
//------------------------------------------------------------------------------
void QtVideo::end_frame() {
	Q_EMIT render_frame();
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
void QtVideo::paintGL() {

	const unsigned int output_width  = width();
	const unsigned int output_height = height();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, output_width, 0, output_height, -1.0, 1.0);
	glViewport(0, 0, output_width, output_height);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, true ? GL_NEAREST : GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, true ? GL_NEAREST : GL_LINEAR);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, Width, Height, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, buffer_);

	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2f(0.0, 0.0); glVertex3i(0, output_height, 0);
	glTexCoord2f(1.0, 0.0); glVertex3i(output_width, output_height, 0);
	glTexCoord2f(0.0, 1.0); glVertex3i(0, 0, 0);
	glTexCoord2f(1.0, 1.0); glVertex3i(output_width, 0, 0);
	glEnd();
}
