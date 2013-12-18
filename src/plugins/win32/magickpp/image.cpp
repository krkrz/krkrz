#include "magickpp.hpp"

// imageをレイヤに描画する
static void Image_display(Magick::Image const *image, iTJSDispatch2* lay) {
	unsigned int w, h;
	unsigned char *p;
	long s;
	if (!lay || TJS_FAILED(lay->IsInstanceOf(0, 0, 0, TJS_W("Layer"), lay)))
		TVPThrowExceptionMessage(TJS_W("Magick::Image: display method needs Layer-instance param."));

	// レイヤサイズ変更
	tTJSVariant tmp[4], *args[4] = { tmp, tmp+1, tmp+2, tmp+3 };
	tmp[0] = true;
	if (TJS_FAILED(lay->PropSet(0, TJS_W("hasImage"), 0, tmp, lay))) goto error;
	tmp[0] = tTVInteger((w = image->columns()));
	tmp[1] = tTVInteger((h = image->rows()));
//	if (TJS_FAILED(lay->FuncCall(0, TJS_W("setSize"),      0, 0, 2, args, lay))) goto error;
	if (TJS_FAILED(lay->FuncCall(0, TJS_W("setImageSize"), 0, 0, 2, args, lay))) goto error;

	// バッファ取得
	lay->PropGet(0, TJS_W("mainImageBufferForWrite"), 0, &tmp[0], lay);
	lay->PropGet(0, TJS_W("mainImageBufferPitch"),    0, &tmp[1], lay);
	p = reinterpret_cast<unsigned char*>((tTVInteger)tmp[0]);
	s = static_cast<long               >((tTVInteger)tmp[1]);

	// コピー
	typedef Magick::PixelPacket PixelT;
	if (sizeof(Magick::Quantum) == 1) {
		// 8bit quantum 専用
		for (unsigned int y = 0; y < h; y++, p+=s) {
			PixelT const *px = image->getConstPixels(0, y, w, 1);
			unsigned char *cp = p;
			for (int i = w; i > 0; i--, px++) {
				*cp++ =  static_cast<unsigned char>(px->blue);		// B
				*cp++ =  static_cast<unsigned char>(px->green);		// G
				*cp++ =  static_cast<unsigned char>(px->red);		// R
				*cp++ = ~static_cast<unsigned char>(px->opacity);	// A
			}
		}
	} else {
		// それ以外はdouble経由なので重い
		for (unsigned int y = 0; y < h; y++, p+=s) {
			PixelT const *px = image->getConstPixels(0, y, w, 1);
			unsigned char *cp = p;
			for (int i = w; i > 0; i--, px++) {
				Magick::ColorRGB col(*px);
				*cp++ = static_cast<unsigned char>(col.blue()  * 255);	// B
				*cp++ = static_cast<unsigned char>(col.green() * 255);	// G
				*cp++ = static_cast<unsigned char>(col.red()   * 255);	// R
				*cp++ = static_cast<unsigned char>(col.alpha() * 255);	// A
			}
		}
	}

	// アップデート
	tmp[0] = tTVInteger(0), tmp[1] = tTVInteger(0);
	tmp[2] = tTVInteger(w), tmp[3] = tTVInteger(h);
	if (TJS_FAILED(lay->FuncCall(0, TJS_W("update"), 0, 0, 4, args, lay))) goto error;
	return;
error:
	TVPThrowExceptionMessage(TJS_W("MagickPP_Image.display failed."));
}

static Magick::Image* ImageCtor_layer(iTJSDispatch2* lay) {
	unsigned int w, h;
	unsigned char *p;
	long s;
	if (!lay || TJS_FAILED(lay->IsInstanceOf(0, 0, 0, TJS_W("Layer"), lay)))
		TVPThrowExceptionMessage(TJS_W("Magick::Image: _layer method needs Layer-instance param."));

	// レイヤサイズ取得
	tTJSVariant tmp[2];
	lay->PropGet(0, TJS_W("imageWidth"),  0, &tmp[0], lay);
	lay->PropGet(0, TJS_W("imageHeight"), 0, &tmp[1], lay);
	w = static_cast<unsigned int>((tTVInteger)tmp[0]);
	h = static_cast<unsigned int>((tTVInteger)tmp[1]);

	// バッファ取得
	lay->PropGet(0, TJS_W("mainImageBuffer"),         0, &tmp[0], lay);
	lay->PropGet(0, TJS_W("mainImageBufferPitch"),    0, &tmp[1], lay);
	p = reinterpret_cast<unsigned char*>((tTVInteger)tmp[0]);
	s = static_cast<long               >((tTVInteger)tmp[1]);

	typedef Magick::PixelPacket PixelT;
	typedef Magick::Color ColorT;
	Magick::Image *image = new Magick::Image(Magick::Geometry(w, h), ColorT());
	image->modifyImage();
	image->type(Magick::TrueColorMatteType);
	// コピー
	if (sizeof(Magick::Quantum) == 1) {
		// 8bit quantum 専用
		for (unsigned int y = 0; y < h; y++, p+=s) {
			unsigned char *cp = p;
			PixelT *q = image->getPixels(0, y, w, 1); 
			for (int i = w; i > 0; i--, cp+=4) {
				*q++ = ColorT(static_cast<Magick::Quantum>(cp[2]),
							  static_cast<Magick::Quantum>(cp[1]),
							  static_cast<Magick::Quantum>(cp[0]),
							  ~static_cast<Magick::Quantum>(cp[3]));
			}
			image->syncPixels();
		}
	} else {
		// それ以外はdouble経由なので重い
		for (unsigned int y = 0; y < h; y++, p+=s) {
			unsigned char *cp = p;
			PixelT *q = image->getPixels(0, y, w, 1); 
			for (int i = w; i > 0; i--, cp+=4) {
				Magick::ColorRGB col(cp[2]/255.0, cp[1]/255.0, cp[0]/255.0);
				col.alpha(cp[3]/255.0);
				*q++ = col;
			}
			image->syncPixels();
		}
	}
	return image;
}

static Magick::Image* ImageCtor_copy(Magick::Image const &cp) { return new Magick::Image(cp); }
static Magick::Image* ImageCtor_geom_col(const Geometry &size_, const Color &color_) { return new Magick::Image(size_, color_); }

// Image
MAGICK_SUBCLASS(Image) {
	NCB_CONSTRUCTOR(());

	Method(TJS_W("_copy"),  ImageCtor_copy);
	Method(TJS_W("_ctor"),  ImageCtor_geom_col);
	Method(TJS_W("_layer"), ImageCtor_layer);

	//////////////////////////////////////////////////////////////////////
	// Image operations

	// Local adaptive threshold image
	// http://www.dai.ed.ac.uk/HIPR2/adpthrsh.htm
	// Width x height define the size of the pixel neighborhood
	// offset = constant to subtract from pixel neighborhood mean
	NCB_METHOD(adaptiveThreshold);

	// Add noise to image with specified noise type
	NCB_METHOD(addNoise);

    // Transform image by specified affine (or free transform) matrix.
//    void            affineTransform ( const DrawableAffine &affine );

	//
	// Annotate image (draw text on image)
	//

#if 0
    // Gravity effects text placement in bounding area according to rules:
    //  NorthWestGravity  text bottom-left corner placed at top-left
    //  NorthGravity      text bottom-center placed at top-center
    //  NorthEastGravity  text bottom-right corner placed at top-right
    //  WestGravity       text left-center placed at left-center
    //  CenterGravity     text center placed at center
    //  EastGravity       text right-center placed at right-center
    //  SouthWestGravity  text top-left placed at bottom-left
    //  SouthGravity      text top-center placed at bottom-center
    //  SouthEastGravity  text top-right placed at bottom-right

    // Annotate using specified text, and placement location
    void            annotate ( const std::string &text_,
             const Geometry &location_ );
    // Annotate using specified text, bounding area, and placement
    // gravity
    void            annotate ( const std::string &text_,
             const Geometry &boundingArea_,
             const GravityType gravity_ );
    // Annotate with text using specified text, bounding area,
    // placement gravity, and rotation.
    void            annotate ( const std::string &text_,
             const Geometry &boundingArea_,
             const GravityType gravity_,
             const double degrees_ );
    // Annotate with text (bounding area is entire image) and placement
    // gravity.
    void            annotate ( const std::string &text_,
             const GravityType gravity_ );
#endif

	// Blur image with specified blur factor
	// The radius_ parameter specifies the radius of the Gaussian, in
	// pixels, not counting the center pixel.  The sigma_ parameter
	// specifies the standard deviation of the Laplacian, in pixels.
	NCB_METHOD(blur);

	// Border image (add border to image)
	NCB_METHOD(border);

	// Extract channel from image
	NCB_METHOD(channel);

    // Set or obtain modulus channel depth
//    void            channelDepth ( const ChannelType channel_,
//                                   const unsigned int depth_ );
//    unsigned int    channelDepth ( const ChannelType channel_ );

	// Charcoal effect image (looks like charcoal sketch)
	// The radius_ parameter specifies the radius of the Gaussian, in
	// pixels, not counting the center pixel.  The sigma_ parameter
	// specifies the standard deviation of the Laplacian, in pixels.
	NCB_METHOD(charcoal);

	// Chop image (remove vertical or horizontal subregion of image)
	// FIXME: describe how geometry argument is used to select either
	// horizontal or vertical subregion of image.
	NCB_METHOD(chop);

#if 0
    // Colorize image with pen color, using specified percent opacity
    // for red, green, and blue quantums
    void            colorize ( const unsigned int opacityRed_,
                               const unsigned int opacityGreen_,
                               const unsigned int opacityBlue_,
             const Color &penColor_ );
    // Colorize image with pen color, using specified percent opacity.
    void            colorize ( const unsigned int opacity_,
             const Color &penColor_ );
#endif

	// Composition operator to be used when composition is implicitly
	// used (such as for image flattening).
	PROP_RW(Magick::CompositeOperator, compose);

	// Compare current image with another image
	// Sets meanErrorPerPixel, normalizedMaxError, and normalizedMeanError
	// in the current image. False is returned if the images are identical.
	NCB_METHOD(compare);

	// Compose an image onto another at specified offset and using
	// specified algorithm
	NCB_METHOD_DETAIL(composite, Class, void, Class::composite, (
		const Image &compositeImage_,
		const int xOffset_,
		const int yOffset_,
		const CompositeOperator compose_));
#if 0
    void            composite ( const Image &compositeImage_,
        const int xOffset_,
        const int yOffset_,
        const CompositeOperator compose_
                                = InCompositeOp );
    void            composite ( const Image &compositeImage_,
        const Geometry &offset_,
        const CompositeOperator compose_
                                = InCompositeOp );
    void            composite ( const Image &compositeImage_,
        const GravityType gravity_,
        const CompositeOperator compose_
                                = InCompositeOp );
#endif
	// Contrast image (enhance intensity differences in image)
	NCB_METHOD(contrast);

	// Convolve image.  Applies a user-specified convolution to the image.
	//  order_ represents the number of columns and rows in the filter kernel.
	//  kernel_ is an array of doubles representing the convolution kernel.
//	NCB_METHOD(convolve);

	// Crop image (subregion of original image)
	NCB_METHOD(crop);

	// Cycle image colormap
	NCB_METHOD(cycleColormap);

	// Despeckle image (reduce speckle noise)
	NCB_METHOD(despeckle);

	// Display image on screen
//	void            display ( void );
	NCB_METHOD_PROXY(display, Image_display);

	// Draw on image using a single drawable
//	NCB_METHOD_DETAIL(draw, Class, void, draw, (Drawable const&));

    // Draw on image using a drawable list
//    void            draw ( const std::list<Magick::Drawable> &drawable_ );

	// Edge image (hilight edges in image)
	NCB_METHOD(edge);

	// Emboss image (hilight edges with 3D effect)
	// The radius_ parameter specifies the radius of the Gaussian, in
	// pixels, not counting the center pixel.  The sigma_ parameter
	// specifies the standard deviation of the Laplacian, in pixels.
	NCB_METHOD(emboss);

	// Enhance image (minimize noise)
	NCB_METHOD(enhance);

	// Equalize image (histogram equalization)
	NCB_METHOD(equalize);

	// Extend the image as defined by the geometry.
	NCB_METHOD(extent);

	// Erase image to current "background color"
	NCB_METHOD(erase);

	// Flip image (reflect each scanline in the vertical direction)
	NCB_METHOD(flip);

#if 0
	// Flood-fill color across pixels
	//   that match the color of the target pixel and are neighbors of the target pixel.
	//   starting at target-pixel and stopping at pixels matching specified border color.
	// Uses current fuzz setting when determining color match.
	void            floodFillColor( const unsigned int x_,
									const unsigned int y_,
									const Color &fillColor_ );
	void            floodFillColor( const Geometry &point_,
									const Color &fillColor_ );
	void            floodFillColor( const unsigned int x_,
									const unsigned int y_,
									const Color &fillColor_,
									const Color &borderColor_ );
	void            floodFillColor( const Geometry &point_,
									const Color &fillColor_,
									const Color &borderColor_ );
#endif

	// Floodfill pixels matching color (within fuzz factor) of target
	// pixel(x,y) with replacement opacity value using method.
	NCB_METHOD(floodFillOpacity);

#if 0
    // Flood-fill texture across pixels that match the color of the
    // target pixel and are neighbors of the target pixel.
    // Uses current fuzz setting when determining color match.
	void            floodFillTexture( const unsigned int x_,
									  const unsigned int y_,
									  const Image &texture_ );
	void            floodFillTexture( const Geometry &point_,
									  const Image &texture_ );

    // Flood-fill texture across pixels starting at target-pixel and
    // stopping at pixels matching specified border color.
    // Uses current fuzz setting when determining color match.
	void            floodFillTexture( const unsigned int x_,
									  const unsigned int y_,
									  const Image &texture_,
									  const Color &borderColor_ );
	void            floodFillTexture( const Geometry &point_,
									  const Image &texture_,
									  const Color &borderColor_ );
#endif
	// Flop image (reflect each scanline in the horizontal direction)
	NCB_METHOD(flop);
#if 0
    // Frame image
	void            frame ( const Geometry &geometry_ = frameGeometryDefault );
	void            frame ( const unsigned int width_,
							const unsigned int height_,
							const int innerBevel_ = 6,
							const int outerBevel_ = 6 );
#endif
	// Gaussian blur image
	// The number of neighbor pixels to be included in the convolution
	// mask is specified by 'width_'. The standard deviation of the
	// gaussian bell curve is specified by 'sigma_'.
	NCB_METHOD(gaussianBlur);

	// Implode image (special effect)
	NCB_METHOD(implode);

	// Level image. Adjust the levels of the image by scaling the
	// colors falling between specified white and black points to the
	// full available quantum range. The parameters provided represent
	// the black, mid (gamma), and white points.  The black point
	// specifies the darkest color in the image. Colors darker than
	// the black point are set to zero. Mid point (gamma) specifies a
	// gamma correction to apply to the image. White point specifies
	// the lightest color in the image.  Colors brighter than the
	// white point are set to the maximum quantum value. The black and
	// white point have the valid range 0 to QuantumRange while mid (gamma)
	// has a useful range of 0 to ten.
	NCB_METHOD(level);

	// Level image channel. Adjust the levels of the image channel by
	// scaling the values falling between specified white and black
	// points to the full available quantum range. The parameters
	// provided represent the black, mid (gamma), and white points.
	// The black point specifies the darkest color in the
	// image. Colors darker than the black point are set to zero. Mid
	// point (gamma) specifies a gamma correction to apply to the
	// image. White point specifies the lightest color in the image.
	// Colors brighter than the white point are set to the maximum
	// quantum value. The black and white point have the valid range 0
	// to QuantumRange while mid (gamma) has a useful range of 0 to ten.
	NCB_METHOD(levelChannel);

	// Magnify image by integral size
	NCB_METHOD(magnify);

	// Remap image colors with closest color from reference image
	NCB_METHOD(map);

	// Floodfill designated area with replacement opacity value
	NCB_METHOD(matteFloodfill);

	// Filter image by replacing each pixel component with the median
	// color in a circular neighborhood
	NCB_METHOD(medianFilter);

	// Reduce image by integral size
	NCB_METHOD(minify);

	// Modulate percent hue, saturation, and brightness of an image
	NCB_METHOD(modulate);

	// Negate colors in image.  Set grayscale to only negate grayscale
	// values in image.
	NCB_METHOD(negate);

	// Normalize image (increase contrast by normalizing the pixel
	// values to span the full range of color values)
	NCB_METHOD(normalize);

	// Oilpaint image (image looks like oil painting)
	NCB_METHOD(oilPaint);

	// Set or attenuate the opacity channel in the image. If the image
	// pixels are opaque then they are set to the specified opacity
	// value, otherwise they are blended with the supplied opacity
	// value.  The value of opacity_ ranges from 0 (completely opaque)
	// to QuantumRange. The defines OpaqueOpacity and TransparentOpacity are
	// available to specify completely opaque or completely
	// transparent, respectively.
	NCB_METHOD(opacity);

	// Change color of opaque pixel to specified pen color.
	NCB_METHOD(opaque);

    // Ping is similar to read except only enough of the image is read
    // to determine the image columns, rows, and filesize.  Access the
    // columns(), rows(), and fileSize() attributes after invoking
    // ping.  The image data is not valid after calling ping.
//    void            ping ( const std::string &imageSpec_ );
    
    // Ping is similar to read except only enough of the image is read
    // to determine the image columns, rows, and filesize.  Access the
    // columns(), rows(), and fileSize() attributes after invoking
    // ping.  The image data is not valid after calling ping.
//    void            ping ( const Blob &blob_ );

	// Quantize image (reduce number of colors)
	NCB_METHOD(quantize);
#if 0
    void            quantumOperator ( const ChannelType channel_,
                                      const MagickEvaluateOperator operator_,
                                      Quantum rvalue_);

    void            quantumOperator ( const int x_,const int y_,
                                      const unsigned int columns_,
                                      const unsigned int rows_,
                                      const ChannelType channel_,
                                      const MagickEvaluateOperator operator_,
                                      const Quantum rvalue_);
#endif
    // Execute a named process module using an argc/argv syntax similar to
    // that accepted by a C 'main' routine. An exception is thrown if the
    // requested process module doesn't exist, fails to load, or fails during
    // execution.
//    void            process ( std::string name_,
//                              const int argc_,
//                              char **argv_ );

	// Raise image (lighten or darken the edges of an image to give a
	// 3-D raised or lowered effect)
	NCB_METHOD(raise);

	// Read single image frame into current object
	NCB_METHOD_DETAIL(read, Class, void, Class::read, (StringT const&));

#if 0
    // Read single image frame of specified size into current object
    void            read ( const Geometry &size_,
         const std::string &imageSpec_ );

    // Read single image frame from in-memory BLOB
    void            read ( const Blob        &blob_ );

    // Read single image frame of specified size from in-memory BLOB
    void            read ( const Blob        &blob_,
         const Geometry    &size_ );

    // Read single image frame of specified size and depth from
    // in-memory BLOB
    void            read ( const Blob         &blob_,
         const Geometry     &size_,
         const unsigned int depth_ );

    // Read single image frame of specified size, depth, and format
    // from in-memory BLOB
    void            read ( const Blob         &blob_,
         const Geometry     &size_,
         const unsigned int depth_,
         const std::string  &magick_ );

    // Read single image frame of specified size, and format from
    // in-memory BLOB
    void            read ( const Blob         &blob_,
         const Geometry     &size_,
         const std::string  &magick_ );

    // Read single image frame from an array of raw pixels, with
    // specified storage type (ConstituteImage), e.g.
    //    image.read( 640, 480, "RGB", 0, pixels );
    void            read ( const unsigned int width_,
                           const unsigned int height_,
                           const std::string &map_,
                           const StorageType  type_,
                           const void        *pixels_ );
#endif

#if 0
    // Reduce noise in image using a noise peak elimination filter
    void            reduceNoise ( void );
    void            reduceNoise ( const double order_ );

    // Roll image (rolls image vertically and horizontally) by specified
    // number of columnms and rows)
    void            roll ( const Geometry &roll_ );
    void            roll ( const unsigned int columns_,
         const unsigned int rows_ );
#endif
	// Resize image to specified size.
	NCB_METHOD(resize);

	// Rotate image counter-clockwise by specified number of degrees.
	NCB_METHOD(rotate);

	// Resize image by using pixel sampling algorithm
	NCB_METHOD(sample);

	// Resize image by using simple ratio algorithm
	NCB_METHOD(scale);

	// Segment (coalesce similar image components) by analyzing the
	// histograms of the color components and identifying units that
	// are homogeneous with the fuzzy c-means technique.  Also uses
	// QuantizeColorSpace and Verbose image attributes
	NCB_METHOD(segment);

	// Shade image using distant light source
	NCB_METHOD(shade);

	// Sharpen pixels in image
	// The radius_ parameter specifies the radius of the Gaussian, in
	// pixels, not counting the center pixel.  The sigma_ parameter
	// specifies the standard deviation of the Laplacian, in pixels.
	NCB_METHOD(sharpen);

	// Shave pixels from image edges.
	NCB_METHOD(shave);

	// Shear image (create parallelogram by sliding image by X or Y axis)
	NCB_METHOD(shear);

	// adjust the image contrast with a non-linear sigmoidal contrast algorithm
	NCB_METHOD(sigmoidalContrast);

	// Solarize image (similar to effect seen when exposing a
	// photographic film to light during the development process)
	NCB_METHOD(solarize);

	// Spread pixels randomly within image by specified ammount
	NCB_METHOD(spread);

	// Add a digital watermark to the image (based on second image)
	NCB_METHOD(stegano);

	// Create an image which appears in stereo when viewed with
	// red-blue glasses (Red image on left, blue on right)
	NCB_METHOD(stereo);

	// Swirl image (image pixels are rotated by degrees)
	NCB_METHOD(swirl);

	// Channel a texture on image background
	NCB_METHOD(texture);

	// Threshold image
	NCB_METHOD(threshold);

    // Transform image based on image and crop geometries
    // Crop geometry is optional
//    void            transform ( const Geometry &imageGeometry_ );
//    void            transform ( const Geometry &imageGeometry_,
//        const Geometry &cropGeometry_  );

	// Add matte image to image, setting pixels matching color to
	// transparent
	NCB_METHOD(transparent);

	// Trim edges that are the background color from the image
	NCB_METHOD(trim);

	// Replace image with a sharpened version of the original image
	// using the unsharp mask algorithm.
	//  radius_
	//    the radius of the Gaussian, in pixels, not counting the
	//    center pixel.
	//  sigma_
	//    the standard deviation of the Gaussian, in pixels.
	//  amount_
	//    the percentage of the difference between the original and
	//    the blur image that is added back into the original.
	// threshold_
	//   the threshold in pixels needed to apply the diffence amount.
	NCB_METHOD(unsharpmask);

	// Map image pixels to a sine wave
	NCB_METHOD(wave);

	// Write single image frame to a file
	NCB_METHOD_DETAIL(write, Class, void, Class::write, (StringT const &));
#if 0
    // Write single image frame to in-memory BLOB, with optional
    // format and adjoin parameters.
    void            write ( Blob *blob_ );
    void            write ( Blob *blob_,
          const std::string &magick_ );
    void            write ( Blob *blob_,
          const std::string &magick_,
          const unsigned int depth_ );

    // Write single image frame to an array of pixels with storage
    // type specified by user (DispatchImage), e.g.
    //   image.write( 0, 0, 640, 1, "RGB", 0, pixels );
    void            write ( const int x_,
                            const int y_,
                            const unsigned int columns_,
                            const unsigned int rows_,
                            const std::string& map_,
                            const StorageType type_,
                            void *pixels_ );
#endif
	// Zoom image to specified size.
	NCB_METHOD(zoom);

	//////////////////////////////////////////////////////////////////////
	// Image Attributes and Options

	// Join images into a single multi-image file
	PROP_RW(bool, adjoin);

	// Anti-alias Postscript and TrueType fonts (default true)
	PROP_rw(bool, antiAlias);

	// Time in 1/100ths of a second which must expire before
	// displaying the next image in an animated sequence.
	PROP_RW(unsigned int, animationDelay);

	// Number of iterations to loop an animation (e.g. Netscape loop
	// extension) for.
	PROP_RW(unsigned int, animationIterations);

	// Access/Update a named image attribute
	NCB_METHOD_DETAIL(getAttribute, Class, StringT, Class::attribute, (StringT));
	NCB_METHOD_DETAIL(setAttribute, Class, void,    Class::attribute, (StringT, StringT));

	// Image background color
	PROP_COLOR(backgroundColor);

	// Name of texture image to tile onto the image background
	PROP_STRING(backgroundTexture);

	// Base image width (before transformations)
	PROP_RO(baseColumns);

	// Base image filename (before transformations)
	PROP_RO(baseFilename);

	// Base image height (before transformations)
	PROP_RO(baseRows);

	// Image border color
	PROP_COLOR(borderColor);

	// Return smallest bounding box enclosing non-border pixels. The
	// current fuzz value is used when discriminating between pixels.
	// This is the crop bounding box used by crop(Geometry(0,0));
	PROP_RO(boundingBox);

	// Text bounding-box base color (default none)
	PROP_COLOR(boxColor);

	// Pixel cache threshold in megabytes.  Once this memory threshold
	// is exceeded, all subsequent pixels cache operations are to/from
	// disk.  This setting is shared by all Image objects.
	NCB_PROPERTY_DETAIL_WO(cacheThreshold, Static, void, Class::cacheThreshold, (unsigned int));

	struct Proxy_chroma {
		static iTJSDispatch2* toDictionary(double x, double y) {
			iTJSDispatch2 *dic = TJSCreateDictionaryObject();
			tTJSVariant var_x(static_cast<tTVReal>(x)), var_y(static_cast<tTVReal>(y));
			dic->PropSet(TJS_MEMBERENSURE, TJS_W("x"), 0, &var_x, dic);
			dic->PropSet(TJS_MEMBERENSURE, TJS_W("y"), 0, &var_y, dic);
			return dic;
		}
		static iTJSDispatch2* getBlue( Image *image) { double x, y; image->chromaBluePrimary( &x, &y); return toDictionary(x, y); }
		static iTJSDispatch2* getGreen(Image *image) { double x, y; image->chromaGreenPrimary(&x, &y); return toDictionary(x, y); }
		static iTJSDispatch2* getRed(  Image *image) { double x, y; image->chromaRedPrimary(  &x, &y); return toDictionary(x, y); }
		static iTJSDispatch2* getWhite(Image *image) { double x, y; image->chromaWhitePoint(  &x, &y); return toDictionary(x, y); }
	};
	// Chromaticity blue primary point (e.g. x=0.15, y=0.06)
	NCB_METHOD_DETAIL(setChromaBluePrimary, Class, void, Class::chromaBluePrimary, ( const double x_, const double y_ ));
	NCB_METHOD_PROXY( getChromaBluePrimary, Proxy_chroma::getBlue);

	// Chromaticity green primary point (e.g. x=0.3, y=0.6)
	NCB_METHOD_DETAIL(setChromaGreenPrimary, Class, void, Class::chromaGreenPrimary, ( const double x_, const double y_ ));
	NCB_METHOD_PROXY( getChromaGreenPrimary, Proxy_chroma::getGreen);

	// Chromaticity red primary point (e.g. x=0.64, y=0.33)
	NCB_METHOD_DETAIL(setChromaRedPrimary, Class, void, Class::chromaRedPrimary, ( const double x_, const double y_ ));
	NCB_METHOD_PROXY( getChromaRedPrimary, Proxy_chroma::getRed);

	// Chromaticity white point (e.g. x=0.3127, y=0.329)
	NCB_METHOD_DETAIL(setChromaWhitePoint, Class, void, Class::chromaWhitePoint, ( const double x_, const double y_ ));
	NCB_METHOD_PROXY( getChromaWhitePoint, Proxy_chroma::getWhite);

	// Image class (DirectClass or PseudoClass)
	// NOTE: setting a DirectClass image to PseudoClass will result in
	// the loss of color information if the number of colors in the
	// image is greater than the maximum palette size (either 256 or
	// 65536 entries depending on the value of QuantumDepth when
	// ImageMagick was built).
	PROP_RW(Magick::ClassType, classType);

	// Associate a clip mask with the image. The clip mask must be the
	// same dimensions as the image. Pass an invalid image to unset an
	// existing clip mask.
	PROP_IMAGE(clipMask);

	// Colors within this distance are considered equal
	PROP_RW(double, colorFuzz);

	// Color at colormap position index_
	NCB_METHOD_DETAIL(getColorMap, Const, Color, Class::colorMap, (unsigned int));
	NCB_METHOD_DETAIL(setColorMap, Class, void,           Class::colorMap, (unsigned int, Color const&));

	// Colormap size (number of colormap entries)
	PROP_rw(unsigned int, colorMapSize);

	// Image Color Space
	PROP_RW(Magick::ColorspaceType, colorSpace);
	PROP_RW(Magick::ColorspaceType, colorspaceType);

	// Image width
	PROP_RO(columns);

	// Image comment
	PROP_STRING(comment);

	// Compression type
	PROP_RW(Magick::CompressionType, compressType);

	// Enable printing of debug messages from ImageMagick
	PROP_RW(bool, debug);

	// Tagged image format define (set/access coder-specific option) The
	// magick_ option specifies the coder the define applies to.  The key_
	// option provides the key specific to that coder.  The value_ option
	// provides the value to set (if any). See the defineSet() method if the
	// key must be removed entirely.
	NCB_METHOD_DETAIL(getDefineValue, Const, StringT, Class::defineValue, (StringT const&, StringT const&));
	NCB_METHOD_DETAIL(setDefineValue, Class, void,    Class::defineValue, (StringT const&, StringT const&, StringT const&));

	// Tagged image format define. Similar to the defineValue() method
	// except that passing the flag_ value 'true' creates a value-less
	// define with that format and key. Passing the flag_ value 'false'
	// removes any existing matching definition. The method returns 'true'
	// if a matching key exists, and 'false' if no matching key exists.
	NCB_METHOD_DETAIL(getDefineSet, Const, bool, Class::defineSet, (StringT const&, StringT const&));
	NCB_METHOD_DETAIL(setDefineSet, Class, void, Class::defineSet, (StringT const&, StringT const&, bool));

	// Vertical and horizontal resolution in pixels of the image
	PROP_GEOMETRY(density);

	// Image depth (bits allocated to red/green/blue components)
	PROP_RW(unsigned int, depth);

	// Tile names from within an image montage
	PROP_RO(directory);

	// Endianness (little like Intel or big like SPARC) for image
	// formats which support endian-specific options.
	PROP_RW(Magick::EndianType, endian);

	// Exif profile (BLOB)
	PROP_BLOB(exifProfile);

	// Image file name
	PROP_STRING(fileName);

	// Number of bytes of the image on disk
	PROP_RO(fileSize);

	// Color to use when filling drawn objects
	PROP_COLOR(fillColor);

	// Rule to use when filling drawn objects
//	PROP_RW_TYPE(Magick::FillRule, fillRule);

	// Pattern to use while filling drawn objects.
	PROP_IMAGE(fillPattern);

	// Filter to use when resizing image
	PROP_RW(Magick::FilterTypes, filterType);

	// Text rendering font
	PROP_STRING(font);

	// Font point size
	PROP_RW(double, fontPointsize);

	// Obtain font metrics for text string given current font,
	// pointsize, and density settings.

	struct Proxy_fontTypeMetrics {
		static TypeMetric* get(Image *image, StringT const &text) {
			TypeMetric *result = new TypeMetric();
			image->fontTypeMetrics(text, result);
			return result;
		}
	};
	NCB_METHOD_PROXY(fontTypeMetrics, Proxy_fontTypeMetrics::get);

	// Long image format description
	PROP_RO(format);

	// Gamma level of the image
//	PROP_RW(double, gamma);
	NCB_METHOD_DETAIL(getGamma,  Const, double, Class::gamma, ());
	NCB_METHOD_DETAIL(setGamma1, Class, void,   Class::gamma, (double));
	NCB_METHOD_DETAIL(setGamma3, Class, void,   Class::gamma, (double, double, double));

	// Preferred size of the image when encoding
	PROP_RO(geometry);

	// GIF disposal method
	PROP_RW(unsigned int, gifDisposeMethod);

	// ICC color profile (BLOB)
	PROP_BLOB(iccColorProfile);

	// Type of interlacing to use
	PROP_RW(Magick::InterlaceType, interlaceType);

	// IPTC profile (BLOB)
	PROP_BLOB(iptcProfile);

	// Does object contain valid image?
	PROP_RW(bool, isValid);

	// Image label
	PROP_STRING(label);

    // Obtain image statistics. Statistics are normalized to the range
    // of 0.0 to 1.0 and are output to the specified ImageStatistics
    // structure.
//    void            statistics ( ImageStatistics *statistics ) const;

	// Stroke width for drawing vector objects (default one)
	// This method is now deprecated. Please use strokeWidth instead.
	PROP_RW(double, lineWidth);

	// File type magick identifier (.e.g "GIF")
	PROP_STRING(magick);

	// Image supports transparency (matte channel)
	PROP_RW(bool, matte);

	// Transparent color
	PROP_COLOR(matteColor);

	// The mean error per pixel computed when an image is color reduced
	PROP_RO(meanErrorPerPixel);

	// Image modulus depth (minimum number of bits required to support
	// red/green/blue components without loss of accuracy)
	PROP_RW(unsigned int, modulusDepth);

	// Tile size and offset within an image montage
	PROP_RO( montageGeometry);

	// Transform image to black and white
	PROP_RW(bool, monochrome);

	// The normalized max error per pixel computed when an image is
	// color reduced.
	PROP_RO(normalizedMaxError);

	// The normalized mean error per pixel computed when an image is
	// color reduced.
	PROP_RO(normalizedMeanError);

	// Image orientation
	PROP_RW(Magick::OrientationType, orientation);

	// Preferred size and location of an image canvas.
	PROP_GEOMETRY(page);

	// Pen color (deprecated, don't use any more)
	PROP_COLOR(penColor);

	// Pen texture image (deprecated, don't use any more)
	PROP_IMAGE(penTexture);

    // Get/set pixel color at location x & y.
	NCB_METHOD_DETAIL(getPixelColor, Const, Color, Class::pixelColor, (unsigned int, unsigned int));
	NCB_METHOD_DETAIL(setPixelColor, Class, void,  Class::pixelColor, (unsigned int, unsigned int, Color const&));

	// Add or remove a named profile to/from the image. Remove the
	// profile by passing an empty Blob (e.g. Blob()). Valid names are
	// "*", "8BIM", "ICM", "IPTC", or a user/format-defined profile name.
	NCB_METHOD_DETAIL(setProfile, Class, void, Class::profile, (StringT, Blob const&));

	// Retrieve a named profile from the image. Valid names are:
	// "8BIM", "8BIMTEXT", "APP1", "APP1JPEG", "ICC", "ICM", & "IPTC"
	// or an existing user/format-defined profile name.
	NCB_METHOD_DETAIL(getProfile, Const, Blob, Class::profile, (StringT));

	// JPEG/MIFF/PNG compression level (default 75).
	PROP_RW(unsigned int, quality);

	// Maximum number of colors to quantize to
	PROP_RW(unsigned int, quantizeColors);

	// Colorspace to quantize in.
	PROP_RW(Magick::ColorspaceType, quantizeColorSpace);

	// Dither image during quantization (default true).
	PROP_RW(bool, quantizeDither);

	// Quantization tree-depth
	PROP_RW(unsigned int, quantizeTreeDepth);

	// The type of rendering intent
	PROP_RW(Magick::RenderingIntent, renderingIntent);

	// Units of image resolution
	PROP_RW(Magick::ResolutionType, resolutionUnits);

	// The number of pixel rows in the image
	PROP_RO(rows);

	// Image scene number
	PROP_RW(unsigned int, scene);

	// Image signature.  Set force_ to true in order to re-calculate
	// the signature regardless of whether the image data has been
	// modified.
	NCB_METHOD(signature);

	// Width and height of a raw image 
	PROP_GEOMETRY(size);

	// enabled/disable stroke anti-aliasing
	PROP_RW(bool, strokeAntiAlias);

	// Color to use when drawing object outlines
	PROP_COLOR(strokeColor);

    // Specify the pattern of dashes and gaps used to stroke
    // paths. The strokeDashArray represents a zero-terminated array
    // of numbers that specify the lengths of alternating dashes and
    // gaps in pixels. If an odd number of values is provided, then
    // the list of values is repeated to yield an even number of
    // values.  A typical strokeDashArray_ array might contain the
    // members 5 3 2 0, where the zero value indicates the end of the
    // pattern array.
//    void            strokeDashArray ( const double* strokeDashArray_ );
//    const double*   strokeDashArray ( void ) const;

	// While drawing using a dash pattern, specify distance into the
	// dash pattern to start the dash (default 0).
	PROP_RW(double, strokeDashOffset);

	// Specify the shape to be used at the end of open subpaths when
	// they are stroked. Values of LineCap are UndefinedCap, ButtCap,
	// RoundCap, and SquareCap.
	PROP_RW(Magick::LineCap, strokeLineCap);
    
	// Specify the shape to be used at the corners of paths (or other
	// vector shapes) when they are stroked. Values of LineJoin are
	// UndefinedJoin, MiterJoin, RoundJoin, and BevelJoin.
	PROP_RW(Magick::LineJoin, strokeLineJoin);

	// Specify miter limit. When two line segments meet at a sharp
	// angle and miter joins have been specified for 'lineJoin', it is
	// possible for the miter to extend far beyond the thickness of
	// the line stroking the path. The miterLimit' imposes a limit on
	// the ratio of the miter length to the 'lineWidth'. The default
	// value of this parameter is 4.
	PROP_RW(unsigned int, strokeMiterLimit);

	// Pattern image to use while stroking object outlines.
	PROP_IMAGE(strokePattern);

	// Stroke width for drawing vector objects (default one)
	PROP_RW(double, strokeWidth);

	// Subimage of an image sequence
	PROP_RW(unsigned int, subImage);

	// Number of images relative to the base image
	PROP_RW(unsigned int, subRange);

	// Annotation text encoding (e.g. "UTF-16")
	PROP_STRING(textEncoding);

	// Tile name
	PROP_STRING(tileName);

	// Number of colors in the image
	PROP_RO(totalColors);

	// Origin of coordinate system to use when annotating with text or drawing
	NCB_METHOD(transformOrigin);

	// Rotation to use when annotating with text or drawing
	PROP_WO(transformRotation);

	// Reset transformation parameters to default
	NCB_METHOD(transformReset);

	// Scale to use when annotating with text or drawing
	NCB_METHOD(transformScale);

	// Skew to use in X axis when annotating with text or drawing
	PROP_WO(transformSkewX);

	// Skew to use in Y axis when annotating with text or drawing
	PROP_WO(transformSkewY);

	// Image representation type
	//   Available types:
	//    Bilevel        Grayscale       GrayscaleMatte
	//    Palette        PaletteMatte    TrueColor
	//    TrueColorMatte ColorSeparation ColorSeparationMatte
	PROP_RW(Magick::ImageType, type);

	// Print detailed information about the image
	PROP_RW(bool, verbose);

	// FlashPix viewing parameters
	PROP_STRING(view);

	// X11 display to display to, obtain fonts from, or to capture
	// image from
	PROP_STRING(x11Display);

	// x resolution of the image
	PROP_RO(xResolution);

	// y resolution of the image
	PROP_RO(yResolution);
}
