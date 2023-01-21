#ifndef CONFIG_H
#define CONFIG_H

/* General */
static constexpr float Tau = 6.28318530718f;
static constexpr float Pi = Tau * .5f;
#define SMOOTH_SEC          0.05f
#define LEFT                0
#define RIGHT               1

/* MS */
#define FILTER_FEEDBACK     0.4f
#define FILTER_DELAY_MS     2.0f
#define PS_DELAY_MS         6.0f

/* EXCITER */
#define HPF_CUTOFF          2000.0f
#define HPF_Q               0.5f
#define DC_CUTOFF           50.0f
#define DC_Q                20.0f
#define OS_FACTOR           1
#define DC_COEFF            0.995

/* PARAMS */
#define GAIN_MIN            -48.0f
#define GAIN_MAX            12.0f
#define GAIN_DEFAULT        0.0f
#define DRIVE_MIN           1.0f
#define DRIVE_MAX           20.0f
#define DRIVE_DEFAULT       1.0f
#define WIDTH_MIN           0.0f
#define WIDTH_MAX           4.0f
#define WIDTH_DEFAULT       1.0f
#define STEREOIZE_MIN       0.0f
#define STEREOIZE_MAX       1.0f
#define STEREOIZE_DEFAULT   0.0f
#define BIAS_MIN            -1.0f
#define BIAS_MAX            1.0f
#define BIAS_DEFAULT        0.0f
#define MIX_MIN             0.0f
#define MIX_MAX             100.0f
#define EXC_MIX_DEFAULT     50.0f

//==============================================================================
/* GUI */
#define WINDOW_WIDTH        800
#define WINDOW_HEIGHT       400
#define FRAME_RATE          60
#define NUM_BLOBS           50
#define BLOB_SIZE_MIN       20
#define BLOB_SIZE_MAX       100
#define STD_FONT_HEIGHT     20

/*
 COLORS
 https://coolors.co/palette/22223b-4a4e69-9a8c98-c9ada7-f2e9e4
*/
#define COLOR_1             34, 34, 59
#define COLOR_2             74, 78, 105
#define COLOR_3             154, 140, 152
#define COLOR_4             201, 173, 167
#define COLOR_5             242, 233, 228


#endif // CONFIG_H
