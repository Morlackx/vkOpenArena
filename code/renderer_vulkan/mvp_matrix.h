#ifndef MVP_MARIX_H_
#define MVP_MARIX_H_

void get_modelview_matrix(float mv[16]);
void set_modelview_matrix(float mv[16]);
void reset_modelview_matrix(void);
void get_mvp_transform(float* mvp);
void myGlMultMatrix( const float *a, const float *b, float *out );


#endif
