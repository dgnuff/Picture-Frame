cbuffer Matrices : register(b0)
{
    matrix viewProjectionMatrix;
};

cbuffer Frames : register(b1)
{
    float4 frames[64];
};

cbuffer DepthBias : register(b2)
{
    float4 depthBias;
};

cbuffer Transforms : register(b3)
{
    float4 transforms[3075];
};

cbuffer PSParams : register(b4)
{
    float4 oldSTFixup;
    float4 newSTFixup;
    float4 ctrlSTFixup;
    float4 frame;
    float4 control;
};

Texture2D oldTexture : register(t0);
Texture2D newTexture : register(t1);
Texture2D controlTexture : register(t2);
Texture2D alphaTexture : register(t3);

SamplerState linearSampler : register(s0);
SamplerState pointSampler : register(s1);

struct VS_INPUT
{
    float4 position : POSITION;
    float2 texcoord0 : TEXCOORD0;
    float2 extra : TEXCOORD1;
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 texcoord0 : TEXCOORD0;
    float2 texcoord1 : TEXCOORD1;
};

float4 aat2pre(uint transformIndex)
{
    float4 trans;
    trans = transforms[transformIndex];
    // Handle projection of a square object to a rectangular viewport
    trans.y = trans.y * depthBias.w;
    return trans;
}

float4 aat2post(uint transformIndex, float frame)
{
    float4 trans;
    // Usually the posttranslate is scaled by the frame number.  That can be done by setting w to 0, causing the
    // lerp to return frame.  If w is set to 1, then unilaterally return 1.  In essence this treats lerp as a
    // C++ ?: operor, just with the three operands written in reverse order.
    float transFrame = lerp(frame, 1.0, transforms[transformIndex].w);
    trans.xyz = transforms[transformIndex].xyz * transFrame;
    // Handle projection of a square object to a rectangular viewport
    trans.y = trans.y * depthBias.w;
    trans.w = 0;
    return trans;
}

float4x4 aat2matrix(uint transformIndex, float frame, float ctrl)
{
    // The first XMFLOAT4 in the Transform is an "axis/angle" value.  The easiest way to describe it is that
    // it's a denormalized quaternion.  In a quaterntion, x, y, z, w are normalized, meaning that you only
    // need to send three of them; you can recover the fourth procedurally.  In this case, x, y, z are a
    // normalized (*) XMFLOAT3, and the angle is free to do whatever the hell it wants.
    // (*) the one exeption to this is that 0, 0, 0 will yeild an identity rotation which does not actually
    // rotate at all, meaning 0, 0, 0, 0 becomes an easliy crafted identity transform.
    // I also transfer all four values because I have an XMFLOAT4 at my disposal, so I might as well use the
    // space to save a bit of computation.
    float x = transforms[transformIndex].x;
    float y = transforms[transformIndex].y;
    float z = transforms[transformIndex].z;
    // Multiply the angle by the frame, this causes the angle to increase over the life of the transition.
    float a = transforms[transformIndex].w * frame;
    float c;
    float s;
    sincos(a, s, c);
    float mc = 1.0f - c;

    // ctrl logic.  The only two values in common use are 0 and -1.  0 will cause ml to always be 1, irrespective
    // of frame, which means the transform is size invarant.  -1 causes ml to shrink from 1 to 0 as frame increases
    // from 0 to 256.  Note that the output value: ml is multipled into the x, y, z components of the first two rows
    // of the output matrix.  This achieves the desired scaling.
    float fc = frame * ctrl / 256.0f;
    float ad = saturate(1.0f - ctrl);
    float ml = fc + ad;

    float4x4 tmatrix;

    // axis / angle to matrix from https://en.wikipedia.org/wiki/Rotation_matrix#Rotation_matrix_from_axis_and_angle
    tmatrix._11 = (c + x * x * mc) * ml;
    tmatrix._12 = (x * y * mc - z * s) * ml;
    tmatrix._13 = (x * z * mc + y * s) * ml;
    tmatrix._14 = 0.0f;

    tmatrix._21 = (x * y * mc + z * s) * ml;
    tmatrix._22 = (c + y * y * mc) * ml;
    tmatrix._23 = (y * z * mc - x * s) * ml;
    tmatrix._24 = 0.0f;

    tmatrix._31 = (x * z * mc - y * s);
    tmatrix._32 = (y * z * mc + x * s);
    tmatrix._33 = (c + z * z * mc);
    tmatrix._34 = 0.0f;

    tmatrix._41 = 0.0f;
    tmatrix._42 = 0.0f;
    tmatrix._43 = 0.0f;
    tmatrix._44 = 1.0f;

    return tmatrix;
}

float getParam(uint index)
{
    // uint offset = index % 4;
    // Fix a bug in Intel HD 4200 hardware.  Using the '%' modulus operator on integers in a vertex shader on
    // this hardware produces incorrect results.
    uint elementIndex = index / 4;
    uint temp = elementIndex * 4;
    uint offset = index - temp;
    float4 element = frames[elementIndex];
    return element[offset];
}

PS_INPUT VSmain(VS_INPUT IN)
{
    PS_INPUT OUT;

    float4 v = float4(IN.position.x, IN.position.y, 0.0f, 1.0f);

    // Handle projection of a square object to a rectangular viewport
    // All incoming y coords are scaled by the viewport aspect ratio.  This is because all of the
    // rendering code thinks it's rendering to a square, so this multiply is necessary to deform
    // that square into a viewport aspect ratio rectangle.
    v.y = v.y * depthBias.w;

    // frame number from [0 .. 256] for transitions, 0 if rendering a static image
    float frame = getParam(round(IN.position.z));

    // Index into transforms for the first XMFLOAT4 of the set of three that make up the desired transform
    uint transformIndex = round((IN.position.w * 1024.0f + depthBias.x) * 3.0f);

    // Fetch the pre translate vector.  By applying this before the main transform, and then undoing after
    // it permits rotation about an arbitrarty axis.  In the context of a typical 4 by 4 XMMATRIX encoding
    // the whole transform, this has the same semantics as the last (translate) row.
    float4 preTranslate = aat2pre(transformIndex + 1);

    // preMult is typically 1.0 - we multiply preTranslate by this value before reversing the translate.
    // By (for example) setting this to 0.0 it allows application of the translate, performing the rotation,
    // and then leaving the results wherever they land.
    float preMult = preTranslate.w;

    // Now we've pulled the multiply out, zero out the w component, and apply the preTranslation
    preTranslate.w = 0.0f;
    v += preTranslate;

    // IN.extra.y is the "ctrl" value, copying a comment from sspf.h that describes it:
    // ctrl == 1: grow over sequence
    // ctrl == 0: size invariant
    // ctrl == -1: shrink over sequence
    // I've never used 1 (yet) and can't think of a use case for it.  0 is the most common, and -1 is useful to
    // get the rendered items to shrink over the course of the transition.
    float4x4 tmatrix = aat2matrix(transformIndex, frame, IN.extra.y);
    v = mul(v, tmatrix);

    // Scale the preTranslate and apply it in reverse
    preTranslate *= preMult;
    v -= preTranslate;

    // Fetch and apply the scaled post translate.
    v += aat2post(transformIndex + 2, frame);

    // Assmble final output
    OUT.position = mul(v, viewProjectionMatrix);
    // Apply depth bias here, rather than faffing around with doing it in render state
    OUT.position.z += depthBias.z + IN.extra.x;
    // tex coords for actual textures and control texture go through unchanged.
    OUT.texcoord0 = IN.texcoord0;
    // tex coords for the control and alpha textures are pulled from the output x,y of the position,
    // this causes them to have screen space lookup semantics.
    // * 0.125f == / (4.0f * 2.0f) == camera distance == horizontal "width" from projection matrix
    OUT.texcoord1 = OUT.position.xy * 0.125 + 0.5;

    return OUT;
}

float4 effect(float4 oldColor, float4 newColor, float select)
{
    // Texture colors map the range [0 .. 255] to [0.0f .. 1.0f].  We don't want this full range, otherwise PEffect transitions
    // tend to break at one end or the other.  Since frame numbers map the range [0 .. 256] to [0.0f .. 1.0f] the correct thing
    // to do is to convert select values from the control texture to the floats that correspond to [0.5 .. 255.5] in the same
    // space.  The multiply and add done here do that conversion.
    select = select * (255.0 / 256.0) + 0.5 / 256.0;
    select = (frame.x + select * control.x + control.y);
    select = saturate(select * control.z + control.w);
    return lerp(oldColor, newColor, select);
}

float4 PSmain(PS_INPUT IN) : SV_Target
{
    float2 texc;
    texc.x = IN.texcoord0.x * oldSTFixup.x + oldSTFixup.y;
    texc.y = IN.texcoord0.y * oldSTFixup.z + oldSTFixup.w;
    float2 texcc = saturate(texc);
    float4 oldColor = oldTexture.Sample(linearSampler, texcc);

    texc.x = IN.texcoord0.x * newSTFixup.x + newSTFixup.y;
    texc.y = IN.texcoord0.y * newSTFixup.z + newSTFixup.w;
    texcc = saturate(texc);
    float4 newColor = newTexture.Sample(linearSampler, texcc);

    texc.x = IN.texcoord1.x * ctrlSTFixup.x;
    texc.y = IN.texcoord1.y * ctrlSTFixup.z;
    texcc = saturate(texc);
    float4 controlColor = controlTexture.Sample(pointSampler, texcc);

    float4 outputColor = effect(oldColor, newColor, controlColor.r);

    float4 alphaColor = alphaTexture.Sample(pointSampler, texcc);

    float diff = frame.y - alphaColor.r;
    float alpha = 1.0 - any(diff);

    //float check = -abs(frame.y - alphaColor.r);
    //float alpha = step(check, 0.0);

    //float alpha = frame.y + alphaColor.r - 1.0;
    //alpha = 1.0 - saturate(abs(alpha));

    outputColor.a = alpha;

    return outputColor;
}
