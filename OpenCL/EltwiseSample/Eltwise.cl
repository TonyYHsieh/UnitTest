
__attribute__((reqd_work_group_size(256, 1, 1))) __kernel void EltSum(global float* out_data,
                                                                      global float* in_data1,
                                                                      global float* in_data2,
                                                                      float coeff1,
                                                                      float coeff2,
                                                                      int count)
{
    int idx = get_global_id(0);
    
    out_data[idx] = coeff1 * in_data1[idx] + coeff2 * in_data2[idx];
}
