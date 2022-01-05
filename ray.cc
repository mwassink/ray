#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <math.h>

typedef unsigned short int u16;
typedef unsigned int u32;
typedef int s32;
typedef float f32;

#define WIDTH 1920
#define HEIGHT 1080
#define AMBIENT .05f
#define EPSILON .01f
#define Assert(expr) if (!(expr)) {*(volatile int*)(0) = 0;}

struct Vec3
{
    f32 x, y, z;
    Vec3(f32 x, f32 y, f32 z) : x(x), y(y), z(z) {};
    Vec3() {x = 0; y = 0; z = 0;}
    Vec3& operator*= (f32 scale) {
        x *= scale; y *= scale; z *= scale;
        return (*this);
    }
    Vec3& operator/= (f32 scale) {
        x /= scale; y /= scale; z /= scale;
        return (*this);
    }

    
}; typedef Vec3 Point;

struct ColorData
{
    f32 r, g, b, a;
    ColorData() {}
    ColorData(f32 r, f32 g, f32 b, f32 a) : r(r), g(g), b(b), a(a) {}
    ColorData& operator*=(f32 f) {
        r *= f; g *= f; b *= f; a *= f;
        return (*this);
    }
    ColorData& operator+=(ColorData& rhs) {
        r += rhs.r; g += rhs.g; b += rhs.b; a += rhs.a;
        return (*this);
    }
};

struct Sphere
{
    ColorData colors;
    Point center;
    f32 radius;    
    Sphere(f32 x,f32 y,f32 z,f32 r) : radius(r) {
        center = Point(x, y, z);
        
    }
    Sphere() {}
};

//implicit plane, normal and 
struct Plane
{
    f32 x, y, z, d;
    ColorData colorChannels;
    Plane(f32 x, f32 y, f32 z, f32 d) : x(x), y(y), z(z), d(d) {}
    Plane() : x(0.0f), y(0.0f), z(0.0f), d(0.0f) {}
    Vec3 getNormal() const {
        return Vec3(x, y, z);
    }
};

struct Triangle
{
    union
    {
        struct
        {
            Point p1, p2, p3;
        };
        struct u32
        {
            s32 i1, i2, i3;
        };
    };
    
};

// Actually more like a sphere
struct PointLight
{
    Point centerLight;
    f32 radius;
    f32 iraddiance; // not cosine corrected right?
    PointLight(f32 x, f32 y, f32 z, f32 r, f32 ir)
    {
        centerLight = Point(x, y, z);
        radius = r;
        iraddiance = ir;
    }
    PointLight() {
        centerLight = {}; radius = 0; iraddiance = 0;
    }
};

struct GeometryData 
{
    Sphere sceneSpheres[100];
    PointLight lights[100];
    Plane planes[10];
    u32 numLights, numSpheres, numPlanes = 0;
    
    
};


// has an origin as well as a vector which we use for the intersection
struct Ray
{
    Point origin;
    Vec3 d;
};

struct Intersect
{
    Point p;
    Vec3 norm;
    ColorData colorChannels;
    s32 valid;
};

#pragma pack(push, 1)
struct BMPFileHeader {
    // bitmap file header
    u16 fileType;
    u32 fileSize;
    u16 reserved1;
    u16 reserved2;
    u32 bitmapOffset;
};

struct BitmapHeader
{
    
    // bitmapheaderCORE
    u32 size;
    s32 width;
    s32 height;
    u16 planes;
    u16 bitsPerPixel; // wraps up the core of the header
    u32 compression;
    u32 imageSize;
    u32 yRes;
    u32 xRes;
    u32 numColorsPalette;
    u32 mostImpColor;
    
    
};

struct Bitmap
{
    BMPFileHeader headFile;
    BitmapHeader head;
};

#pragma pack(pop)


f32 mag(const Vec3& in)
{
    return sqrt(in.x * in.x + in.y * in.y + in.z * in.z);
}

f32 dot(const Vec3& a, const Vec3& b) {
    return (a.x* b.x + a.y * b.y + a.z * b.z);
}

f32 absf(f32 f) {
    if (f < 0) return f * -1;
    return f;
}

f32 clamp(f32 in) {
    if (in < 0.0f) return 0.0f;
    if (in > 1.0f) return 1.0f;
    return in;
}

//computes b - a
Vec3 operator-(const Vec3& b, const Vec3& a)
{
    return Vec3(b.x - a.x, b.y-a.y, b.z - a.z);
}
Vec3 operator+(const Vec3& b, const Vec3& a)
{
    return Vec3(b.x + a.x, b.y+a.y, b.z + a.z);
}
Vec3 operator*(const Vec3& b, const Vec3& a)
{
    return Vec3(b.x * a.x, b.y*a.y, b.z * a.z);
}

Vec3 operator*(const Vec3& v, f32 f) {
    return Vec3(v.x*f, v.y * f, v.z * f);
}


Vec3 operator-(const Vec3& v, f32 f) {
    return Vec3(v.x-f, v.y - f, v.z - f);
}


Vec3 operator+(const Vec3& v, f32 f) {
    return Vec3(v.x+f, v.y + f, v.z + f);
}


Vec3 operator/(const Vec3& v, f32 f) {
    return Vec3(v.x/f, v.y / f, v.z / f);
}

Vec3 normalize(const Vec3& v)
{
    return (v / mag(v));
}


Vec3 ReflectAcrossNorm(const Vec3& v, const Vec3& n ) {
    // Project v onto n
    Vec3 proj =  n * dot(v, n)/dot(n, n);
    Vec3 negRejection =  (v - proj)*-1;
    return proj + negRejection;
}


ColorData operator+(const ColorData& l, const ColorData& r) {
    return ColorData(l.r + r.r, l.g + r.g, l.b + r.b, l.a + r.a);
}


ColorData operator*(const ColorData& l, f32 f) {
    return ColorData(l.r * f, l.g * f, l.b * f, l.a * f);
}

// Returns 10 hard coded spheres for this
GeometryData GenerateSpheres(void)
{
    GeometryData data;
    ColorData ball3Color(1.0f, 0.0f, 0.0f, 1.0f);
    ColorData ball2Color(1.0f, 0.0f, 1.0f, 1.0f);
    ColorData planeColor(1.0f, 0.0f, 0.0f, 1.0f);
    ColorData planeColor2(0.0f, 1.0f, 0.0f, 1.0f);
    data.sceneSpheres[0] = Sphere(7.0f, -23.0f, 40.0f, 7.0f);
    data.sceneSpheres[0].colors = ball3Color;
    data.sceneSpheres[1] = Sphere(15.0f, -24.0f, 80.0f, 6.0f);
    data.sceneSpheres[1].colors = ball2Color;
    data.sceneSpheres[2] = Sphere(19.0f, -29.0f, 19.0f, .5f);
    data.sceneSpheres[2].colors = ball3Color;
    data.sceneSpheres[3] = Sphere(-4.0f, -23.0f, 40.0f, 4.0f);
    data.sceneSpheres[3].colors = ball3Color;
    
    data.lights[0] = PointLight(20.0f, 20.0f, -20.0f, 2.0f, 100000.0f);
    
    data.planes[0] = Plane(0.0f, 0.0f, -1.0f, 100.0f);
    data.planes[1] = Plane(0.0f, 1.0f, 0.0f, 30.0f);
    data.planes[2] = Plane(0.0f, -1.0f, 0.0f, 30.0f);
    data.planes[3] = Plane(1.0f, 0.0f, 0.0f, 30.0f);
    data.planes[4] = Plane(-1.0f/sqrt(5.0f), -2.0f/sqrt(5.0f), 0.0f, 30.0f);
    data.planes[0].colorChannels = planeColor;
    data.planes[1].colorChannels = planeColor2;
    data.planes[2].colorChannels = planeColor2;
    data.planes[3].colorChannels = planeColor2;
    data.planes[4].colorChannels = planeColor2;

    data.numSpheres = 4;
    data.numLights = 1;
    data.numPlanes = 5;
    return data;
    
}

// Let's have this attenuate (is that word used for dropoff or only for full dropoff?)like a sphere based on SA
f32 SphericalLightAttenuate(const PointLight& light, const Point& atPoint)
{
    Vec3 diff = light.centerLight - atPoint;
    f32 rad2 = dot(diff, diff);
    f32 sa  = 4 * rad2 * 3.14;
    return (light.iraddiance / sa);
    
}

// This will give some sort of relative lighting term... still need to see how much it affects
// (TODO) do not assume that the incoming light color is white
f32 DiffuseLightingContrib(const PointLight& light, const Vec3& norm, const Point& pt)
{
    Vec3 vlight = light.centerLight - pt;
    f32 ndotl = dot(normalize(vlight), norm);
    if (ndotl < 0) {
        return 0.0f;
    }
    Assert(ndotl < 1.01);
    f32 lightIn = SphericalLightAttenuate(light, pt);
    return (lightIn * ndotl / 3.14);
}

// phong
// (TODO) do not assume that the incoming light is white
f32 SpecularLightingContrib(const PointLight& light, const Vec3& norm, const Vec3& v, const Point& pt, f32 shininess)
{
    Vec3 l = light.centerLight - pt; //might actually be -l conventionally
    l = normalize(l);
    Vec3 r = ReflectAcrossNorm(l, norm);
    Vec3 v_norm = normalize(v);
    
    f32 specAngle = dot(r, v_norm);
    if (specAngle < 0) { return 0; }
    Assert(specAngle < 1.01f);
    f32 specScale = pow(specAngle, shininess);
    f32 lightIn = SphericalLightAttenuate(light, pt);
    return (lightIn * specScale);
}



// Pack the color for the storing in the bitmap
u32 PackColorInternal(f32 r_f, f32 g_f, f32 b_f, f32 a_f ) {
    unsigned char r, g, b, a;
    r_f = clamp(r_f); g_f = clamp(g_f); b_f = clamp(b_f); a_f = clamp(a_f);
    r = r_f*255; 
    g = g_f*255;  
    b = b_f*255; 
    a = a_f*255;
    
    u32 target = (a << 24) | (r << 16) | (g << 8) | (b); // this will go rgba when it is put into memory
    return target;
}

u32 PackColor(const ColorData& colors)
{
    return PackColorInternal(colors.r, colors.g ,colors.b, colors.a);
}

Vec3 NormalPointOnSphere(const Sphere& s, const Point& p )
{
    return (normalize(p - s.center));
}







f32 TestRaySphere(const Ray& ray, const Sphere& sphere, Intersect* intersect)
{
    f32 t;
    f32 r2 = sphere.radius * sphere.radius;
    Point lpoint = sphere.center - ray.origin; //the 
    f32 s = dot(lpoint, ray.d); //d should be normalized
    f32 ll = dot(lpoint, lpoint);
    if (s < 0 && ll > r2) return -1; //behind AND ray origin is outside of the sphere
    f32 m2 = ll - s*s; //squared distance to the projection
    if (m2 > r2) return -1; // if squared dist to proj is greater than r2 return
    
    f32 q = sqrt(r2 - m2); //other leg of the triangle 
    //are we outside the sphere?
    if (ll > r2)
        t = s - q;
    else
        t = s + q; // inside the sphere
    
    intersect->p = ray.origin + (ray.d * t);
    intersect->norm = NormalPointOnSphere(sphere, intersect->p);
    intersect->colorChannels = sphere.colors;
    intersect->valid = 1;
    return mag(intersect->p - ray.origin);
    
    
}

f32 TestRayPlane(const Ray& ray, const Plane& plane, Intersect* intersect)
{
    // Parallel check
    Vec3 normalPlane = plane.getNormal();
    f32 proj = dot(ray.d, normalPlane);
    if (absf(proj) < EPSILON) {
        intersect->valid = 0;
        return -1;
    }
    f32 t = (-dot(ray.origin, normalPlane) - plane.d)/(dot(ray.d, normalPlane));
    if (t < 0) {
        intersect->valid = 0;
        return -1;
    }
    intersect->p = ray.origin + (ray.d * t);
    intersect->norm = normalPlane;
    intersect->colorChannels =  plane.colorChannels;
    intersect->valid = 1;
    return mag(intersect->p - ray.origin);
 }


Intersect FindClosestIntersection(const Ray& eyeRay, const GeometryData* sceneData, f32 maxDistance = 200000.0f)
{
   
    s32 numSpheres = sceneData->numSpheres;
    s32 numPlanes = sceneData->numPlanes;
    Intersect currentIntersect, bestIntersect;
    bestIntersect.valid = 0;
    f32 bestDist = 200000.0f;
    // Spheres
    for (int i = 0; i < numSpheres; ++i) {
        f32 dist = TestRaySphere(eyeRay, sceneData->sceneSpheres[i], &currentIntersect);
        if (dist > EPSILON && dist < bestDist && dist < maxDistance) {
            bestDist = dist;
            bestIntersect = currentIntersect;
        }
    }
    // Planes
    for (int i = 0; i < numPlanes; ++i) {
        f32 dist = TestRayPlane(eyeRay, sceneData->planes[i], &currentIntersect);
        if (dist > EPSILON && dist < bestDist && dist < maxDistance) {
            bestDist = dist;
            bestIntersect = currentIntersect;
        }
    }
    return bestIntersect;
}

u32 Shade(const GeometryData* data, Intersect& intersect)
{
    const f32 shininess = 16.0f;
    // need to go through all of the lights and all of the spheres and see if we can hit the light
    ColorData finalColor(0.0f, 0.0f, 0.0f, 1.0f);
    ColorData ambient(.05f, .05f, .05f, 1.0f);
    ColorData specIn(1.0f, 1.0f, 1.0f, 1.0f);
    for (int i = 0; i < data->numLights; ++i) {
        // loop over all of the geometry for a collision
        Ray shadowRay;
        shadowRay.origin = intersect.p;
        shadowRay.d = data->lights[i].centerLight - intersect.p;
        f32 maxDist = mag(data->lights[i].centerLight - intersect.p); // For clarity
        shadowRay.d /= maxDist;
        Intersect shadowIntersect = FindClosestIntersection(shadowRay, data, maxDist);
        if (shadowIntersect.valid) {
            finalColor += ambient;
        }
        else {
            f32 diff = DiffuseLightingContrib(data->lights[i], intersect.norm, intersect.p);
            f32 specular = SpecularLightingContrib(data->lights[i], intersect.norm, intersect.p * (-1), intersect.p, shininess);
            ColorData diffColor = intersect.colorChannels * diff;
            ColorData specOut = specIn * specular;
            finalColor += (diffColor + specOut + ambient);
        }
    }


    return PackColor(finalColor);
    //return 0x0000FF00; // x r g b
}



u32 TraceRay(const Ray& ray, const GeometryData* scene)
{
    u32 color = 0x0;
    Intersect intersect = FindClosestIntersection(ray, scene);
    return Shade(scene, intersect);
}

Ray DirFromPixelXY(s32 pixelX, s32 pixelY, s32  width, s32 height, f32 vertFOV)
{
    // Do this as a grid where we have the z value at the back set by the field of view that we want
    // (0, 0) be the center
    f32 z = (height/2) / tanf(vertFOV /2);
    f32 x = -(width/2) + pixelX + .5f;
    f32 y = -(height/2) + pixelY + .5f;
    Point v(x, y, z);
    v = v /= mag(v);
    Ray ray;
    ray.origin  = Vec3(0.0f, 0.0f, 0.0f);
    ray.d = v;
    return ray;
}

// Partial derivative with respect to t?
u32 ColorPartial(Ray ray) {
    unsigned char r, g, b, a;
    r = absf(ray.d.x)*255; 
    g = absf(ray.d.y)*255;  
    b = absf(ray.d.z)*255; 
    a = 1;
    
    u32 target = (a << 24) | (b << 16) | (g << 8) | (r); // this will go rgba when it is put into memory
    return target;
}
void TraceImage(s32 width, s32 height, f32 vertFOV, u32* colors, const GeometryData* sceneData)
{
    for (s32 y = 0; y < height; ++y) {
        for (s32 x = 0; x < width; ++x) {
            Ray ray = DirFromPixelXY(x, y, width, height, vertFOV);
            u32 color = TraceRay(ray, sceneData);
            colors[y*width + x] = color;
        }
    }
}

void WriteBMPHeader(FILE* fp, u32 height, u32 width)
{
    Bitmap bmp;
    BMPFileHeader fileHeader = {};
    
    fileHeader.fileType = 0x4D42;
    fileHeader.fileSize = sizeof(Bitmap) + (width * height*sizeof(u32));
    fileHeader.bitmapOffset = sizeof(Bitmap); //ends the file header
    fileHeader.reserved1 = 0;
    fileHeader.reserved2 = 0;
    
    BitmapHeader header = {};
    header.size = sizeof(header); // minus the first five fields
    header.width = width;
    header.height = height;
    header.planes = 1;
    header.bitsPerPixel = 32; //rgba
    header.compression = 0;
    header.imageSize = height* width * 4;
    header.xRes = 50;
    header.yRes = 50;
    header.numColorsPalette = 0;
    
    bmp.headFile = fileHeader;
    bmp.head = header;
    
    fwrite((void*)&bmp, 1, sizeof(Bitmap), fp);
    
}

void WriteBMP(void* data, u32 height, u32 width)
{
    FILE* fp = fopen("test.bmp", "wb");
    WriteBMPHeader(fp, height, width);
    fwrite(data, sizeof(u32), height* width,fp );
    
    
}

int main(int argCount, char** argv)
{
    GeometryData sceneData = GenerateSpheres();
    u32 outputWidth = WIDTH;
    u32 outputHeight = HEIGHT;
    u32* data = (u32*) malloc(outputHeight* outputWidth * sizeof(u32));
    memset(data, 0x80, outputWidth* outputHeight * sizeof(u32));
    
    TraceImage(WIDTH, HEIGHT, 3.14f / 2.0f, data, &sceneData  );
    WriteBMP(data, outputHeight, outputWidth);
}
