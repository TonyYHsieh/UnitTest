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

