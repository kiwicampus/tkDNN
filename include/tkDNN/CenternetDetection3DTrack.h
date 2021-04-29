#ifndef CENTERNETDETECTION3DTRACK_H
#define CENTERNETDETECTION3DTRACK_H

#include "kernels.h"
#include "utils.h"
#include "tkdnn.h"
#include <opencv2/videoio.hpp>
#include "opencv2/opencv.hpp"
#include <time.h>
#include <vector>
#include <numeric>      // std::iota
#include <algorithm>    // std::sort

#include "DetectionNN3D.h"

#include "kernelsThrust.h"


namespace tk { namespace dnn { 

struct detectionRes
{
    float score;
    int cl;
    cv::Mat ct, tr, bb0, bb1;
    float dep;
    float dim[3];
    float alpha;
    float x,y,z;
    float rot_y;
    detectionRes() : ct(cv::Mat(cv::Size(1,2), CV_32F)),
                          tr(cv::Mat(cv::Size(1,2), CV_32F)),
                          bb0(cv::Mat(cv::Size(1,2), CV_32F)),
                          bb1(cv::Mat(cv::Size(1,2), CV_32F)) { }
    ~detectionRes() {
        ct.release();
        tr.release();
        bb0.release();
        bb1.release();
    }
};

struct trackingRes
{
    struct detectionRes det_res;
    int tracking_id;
    int age;
    int active;
    int color;
};

class CenternetDetection3DTrack : public DetectionNN3D
{
private:
    tk::dnn::dataDim_t dim;
    tk::dnn::dataDim_t dim2;
    tk::dnn::dataDim_t dim_hm; 
    tk::dnn::dataDim_t dim_wh; 
    tk::dnn::dataDim_t dim_reg;
    tk::dnn::dataDim_t dim_track;
    tk::dnn::dataDim_t dim_dep;
    tk::dnn::dataDim_t dim_rot;
    tk::dnn::dataDim_t dim_dim;
    tk::dnn::dataDim_t dim_amodel_offset;

    /* preprocessing */
    #ifdef OPENCV_CUDACONTRIB
        float *mean_d;
        float *stddev_d;
    #else
        cv::Vec<float, 3> mean;
        cv::Vec<float, 3> stddev;
        dnnType *input;
    #endif
    float *d_ptrs;

    cv::Mat src;
    cv::Mat dst;
    cv::Mat dst2;  
    cv::Mat trans, trans2, trans_out;

    /* pre inf */
    bool iter0;
    dnnType *input_pre_inf_d;
    bool test_pre_inf = true;
    dnnType *img_d, *hm_d;
    tk::dnn::dataDim_t dim_in0;
    tk::dnn::dataDim_t dim_in1;
    dnnType *out_d;


    /* postprocessing */
    int K = 100;
    int width = 128;//56;        // TODO

    // pointer used in the kernels
    float *src_out;
    int *ids_out;

    float *topk_scores;
    int *topk_inds_;
    float *topk_ys_;
    float *topk_xs_;
    int *ids_d, *ids_;

    float *ones;

    float *scores, *scores_d;
    int *clses, *clses_d; 
    int *topk_inds_d;
    float *topk_ys_d; 
    float *topk_xs_d;
    int *inttopk_xs_d, *inttopk_ys_d;
    
    float *bbx0, *bby0, *bbx1, *bby1;
    float *bbx0_d, *bby0_d, *bbx1_d, *bby1_d;

    int *intxs, *intys;

    float *track, *dep, *rot, *dim_, *wh, *amodel_offset;
    float *track_d, *dep_d, *rot_d, *dim_d, *wh_d, *amodel_offset_d;
    
    float *target_coords;

    /* visualization */
    cv::Mat r;
    cv::Mat calibs;
    cv::Mat corners, pts3DHomo;  

    std::vector<std::vector<int>> face_id;
    cv::Scalar tr_colors[256];
    bool view2d = false;

    //processing
    struct threshold op;
    float out_thresh = 0.1;
    float new_thresh = 0.3;
    float vis_thresh = 0.3;
    float peakThreshold = 0.2;
    float centerThreshold = 0.3; //default 0.5
    

    //detections
    std::vector<struct detectionRes> det_res;
    int count_det;
    //tracks
    std::vector<struct trackingRes> tr_res;
    std::vector<std::vector<struct trackingRes>> batchTracked;
    int count_tr;
    int track_id=0;
  
    
    bool init_preprocessing();
    bool init_pre_inf();
    bool init_postprocessing();
    bool init_visualization(const int n_classes);
    void pre_inf(const int bi);
    void _get_additional_inputs();
    cv::Mat transform_preds_with_trans(float x1, float x2);
    void tracking();

public:
    tk::dnn::Network *pre_phase_net = nullptr;
    CenternetDetection3DTrack() {};
    ~CenternetDetection3DTrack() {};
    bool init(const std::string& tensor_path, const int n_classes=3, const int n_batches=1, const float conf_thresh=0.3);
    void preprocess(cv::Mat &frame, const int bi=0);
    void postprocess(const int bi=0,const bool mAP=false);
    void draw(std::vector<cv::Mat>& frames);
};


} // namespace dnn
} // namespace tk


#endif /*CENTERNETDETECTION3DTRACK_H*/