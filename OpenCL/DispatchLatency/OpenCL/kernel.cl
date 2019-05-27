kernel void NearlyNull1(global float* a, global float* b, global float* c, global float* d, int t)
{
    if (a) {
        a[0] = 42;
    }

    if (b) {
        b[t] = 50;
    }

    if (c) {
        c[t] = 51;
    }

    if (d) {
        d[t] = 52;
    }
}

kernel void NearlyNull2(global float* a, global float* b, global float* c, global float* d, int t)
{
    if (a) {
        a[0] += 100;
    }

    if (b) {
        b[0] += 200;
    }

    if (c) {
        c[0] += 300;
    }

    if (d) {
        d[0] += 400;
    }
}

kernel void NearlyNull3(global float* a, global float* b, global float* c, global float* d, int t)
{
    if (a) {
        a[0] *= 100;
    }

    if (b) {
        b[0] *= 200;
    }

    if (c) {
        c[0] *= 300;
    }

    if (d) {
        d[0] *= 400;
    }
}

kernel void NearlyNull4(global float* a, global float* b, global float* c, global float* d, int t)
{
    if (a) {
        a[0] -= 100;
    }

    if (b) {
        b[0] -= 200;
    }

    if (c) {
        c[0] -= 300;
    }

    if (d) {
        d[0] -= 400;
    }
}

kernel void NearlyNull5(global float* a, global float* b, global float* c, global float* d, int t)
{
    if (a) {
        a[0] /= 100;
    }

    if (b) {
        b[0] /= 200;
    }

    if (c) {
        c[0] /= 300;
    }

    if (d) {
        d[0] /= 400;
    }
}

kernel void NearlyNull6(global float* a, global float* b, global float* c, global float* d, int t)
{
    if (a) {
        a[0] += 100*t;
    }

    if (b) {
        b[0] += 200*t;
    }

    if (c) {
        c[0] += 300*t;
    }

    if (d) {
        d[0] += 400*t;
    }
}

kernel void NearlyNull7(global float* a, global float* b, global float* c, global float* d, int t)
{
    if (a) {
        a[0] *= 100*t;
    }

    if (b) {
        b[0] *= 200*t;
    }

    if (c) {
        c[0] *= 300*t;
    }

    if (d) {
        d[0] *= 400*t;
    }
}

kernel void NearlyNull8(global float* a, global float* b, global float* c, global float* d, int t)
{
    if (a) {
        a[0] = sin(100.0f);
    }

    if (b) {
        b[0] = sin(200.0f);
    }

    if (c) {
        c[0] = sin(300.0f);
    }

    if (d) {
        d[0] = sin(400.0f);
    }
}

kernel void NearlyNull9(global float* a, global float* b, global float* c, global float* d, int t)
{
    if (a) {
        a[0] = tanh(100.0f);
    }

    if (b) {
        b[0] = tanh(200.0f);
    }

    if (c) {
        c[0] = tanh(300.0f);
    }

    if (d) {
        d[0] = tanh(400.0f);
    }
}

kernel void NearlyNull10(global float* a, global float* b, global float* c, global float* d, int t)
{
    if (a) {
        a[0] = exp(100.0f);
    }

    if (b) {
        b[0] = exp(200.0f);
    }

    if (c) {
        c[0] = exp(300.0f);
    }

    if (d) {
        d[0] = exp(400.0f);
    }
}
