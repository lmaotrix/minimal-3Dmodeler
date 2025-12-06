# version 430

uniform vec3 camPos;
uniform vec3 camTarget;
uniform vec2 resolution;
uniform float utime;

out vec4 fragColor;

float sdSphere(vec3 p, float r)
{
    return length(p) - r;
}

float sceneSDF(vec3 p)
{
    return sdSphere(p, 1.0);
}

vec3 rayDirection(float fov, vec2 fragCoord, vec2 res)
{
    vec2 xy = fragCoord - res * 0.5;
    float z = res.y / tan(radians(fov) / 2.0);
    return normalize(vec3(xy, -z));
}

vec3 estimateNormal(vec3 p)
{
    float d = sceneSDF(p);
    float eps = 0.001;
    vec3 n = vec3(
        sceneSDF(p + vec3(eps, 0, 0)) - d,
        sceneSDF(p + vec3(0, eps, 0)) - d,
        sceneSDF(p + vec3(0, 0, eps)) - d
    );
    return normalize(n);
}

void main()
{
    vec2 uv = gl_FragCoord.xy;

    vec3 f = normalize(camTarget - camPos);
    vec3 r = normalize(cross(f, vec3(0, 1, 0)));
    vec3 u = cross(f,r);

    vec3 rd = rayDirection(60.0, uv, resolution);
    rd = rd.x * r + rd.y * u + rd.z * f;

    vec3 ro = camPos;

    float t = 0.0;
    float MAX_DIST = 50.0;
    float EPS = 0.001;

    for (int i = 0; i < 200; i++)
    {
        vec3 p = ro + rd * t;
        float d = sceneSDF(p);
        if (d < EPS) break;
        t += d;
        if (t > MAX_DIST) break;
    }

    if (t > MAX_DIST)
    {
        fragColor = vec4(0.0, 0.0, 0.0, 1.0); // background
        return;
    }

    vec3 hit = ro + rd * t;
    vec3 n = estimateNormal(hit);

    vec3 lightDir = normalize(vec3(1, 1, -1));
    float diff = max(dot(n, lightDir), 0.0);

    fragColor = vec4(vec3(diff), 1.0);
}
