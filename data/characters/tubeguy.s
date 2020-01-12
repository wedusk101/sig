# SIG Skeleton Definition

KnSkeleton

name tubeguy

geofile "tubeguy.m"

skeleton
root Hips
{ offset 0 0 0
  channel XPos 0 free
  channel YPos 0 free
  channel ZPos 0 free
  channel Quat

  joint Spine
  { offset 0 4.46517 -0.102243
    visgeo "tubeguy_spine_vis.m"
    channel Quat

    joint Spine1
    { offset 0 4.46212 -0.182402
      visgeo "tubeguy_spine1_vis.m"
      channel Quat

      joint Spine2
      { offset 0 4.37937 -0.881002
        visgeo "tubeguy_spine2_vis.m"
        channel Quat

        joint Spine3
        { offset 0 4.68286 0.214145
          channel Quat

          joint Neck
          { offset 0 1.76964 0.824538
            visgeo "tubeguy_neck_vis.m"
            channel Quat

            joint Head
            { offset 0 1.6961 0.931451
              visgeo "tubeguy_head_vis.m"
              channel Quat
            }
          }

          joint LeftShoulder
          { offset 1.19381 -0.758808 -0.0276761
            visgeo "tubeguy_leftshoulder_vis.m"
            channel Quat

            joint LeftArm
            { offset 5.18779 -1.70971e-05 0
              visgeo "tubeguy_leftarm_vis.m"
              channel Swing lim 110 150
              channel Twist 0 lim -90 120
              prerot axis 0 1 0 ang 90
              postrot axis 0 -1 0 ang 90

              joint LeftForeArm
              { offset 12.7364 -3.64212e-05 1.60049e-05
                visgeo "tubeguy_leftforearm_vis.m"
                euler ZY
                channel YRot 0 lim -160 0
                channel ZRot 0 lim -45 120
                prerot axis 0 1 0 ang 90
                postrot axis 0 -1 0 ang 90

                joint LeftHand
                { offset 9.21452 -2.35898e-05 1.16309e-05
                  visgeo "tubeguy_lefthand_vis.m"
                  channel Swing lim 50 90
                  prerot axis 0 1 0 ang 90
                  postrot axis 0 -1 0 ang 90
                }
              }
            }
          }

          joint RightShoulder
          { offset -1.19381 -0.758806 -0.0276721
            visgeo "tubeguy_rightshoulder_vis.m"
            channel Quat

            joint RightArm
            { offset -5.18779 1.32818e-05 0
              visgeo "tubeguy_rightarm_vis.m"
              channel Swing lim 110 150
              channel Twist 0 lim -90 120
              prerot axis 0 -1 0 ang 90
              postrot axis 0 1 0 ang 90

              joint RightForeArm
              { offset -12.7364 3.26084e-05 -1.56275e-05
                visgeo "tubeguy_rightforearm_vis.m"
                euler ZY
                channel YRot 0 lim 0 160
                channel ZRot 0 lim -120 45
                prerot axis 0 -1 0 ang 90
                postrot axis 0 1 0 ang 90

                joint RightHand
                { offset -9.21452 2.35923e-05 -1.1154e-05
                  visgeo "tubeguy_righthand_vis.m"
                  channel Swing lim 50 90
                  prerot axis 0 -1 0 ang 90
                  postrot axis 0 1 0 ang 90
                }
              }
            }
          }
        }
      }
    }
  }

  joint LeftUpLeg
  { offset 4.06525 -3.90732 0.417113
    visgeo "tubeguy_leftupleg_vis.m"
    channel Swing lim 65 120
    channel Twist 0 lim -100 30
    prerot axis 0.57735 0.57735 -0.57735 ang 120
    postrot axis -0.57735 -0.57735 0.57735 ang 120

    joint LeftLeg
    { offset 0.000225239 -18.4821 0.0036693
      visgeo "tubeguy_leftleg_vis.m"
      euler ZY
      channel YRot 0 lim 0 160
      channel ZRot 0 lim -55 55
      prerot axis 0.57735 0.57735 -0.57735 ang 120
      postrot axis -0.57735 -0.57735 0.57735 ang 120

      joint LeftFoot
      { offset -0.00172431 -15.2898 -0.0262949
        visgeo "tubeguy_leftfoot_vis.m"
        channel Swing lim 35 60
        prerot axis 0.57735 0.57735 -0.57735 ang 120
        postrot axis -0.57735 -0.57735 0.57735 ang 120

        joint LeftToeBase
        { offset 0.367432 -3.19131 5.54287
          visgeo "tubeguy_lefttoebase_vis.m"
          channel Quat

          joint LeftToe_End
          { offset 0.0875082 0.0484885 1.36758
            channel Quat
          }
        }
      }
    }
  }

  joint RightUpLeg
  { offset -4.06526 -3.90731 0.417124
    visgeo "tubeguy_rightupleg_vis.m"
    channel Swing lim 65 120
    channel Twist 0 lim -30 100
    prerot axis 0.57735 0.57735 -0.57735 ang 120
    postrot axis -0.57735 -0.57735 0.57735 ang 120

    joint RightLeg
    { offset -0.000293929 -18.4821 0.00440263
      visgeo "tubeguy_rightleg_vis.m"
      euler ZY
      channel YRot 0 lim 0 160
      channel ZRot 0 lim -55 55
      prerot axis 0.57735 0.57735 -0.57735 ang 120
      postrot axis -0.57735 -0.57735 0.57735 ang 120

      joint RightFoot
      { offset 0.00159742 -15.2898 -0.0256927
        visgeo "tubeguy_rightfoot_vis.m"
        channel Swing lim 35 60
        prerot axis 0.57735 0.57735 -0.57735 ang 120
        postrot axis -0.57735 -0.57735 0.57735 ang 120

        joint RightToeBase
        { offset -0.354204 -3.19109 5.54386
          visgeo "tubeguy_righttoebase_vis.m"
          channel Quat

          joint RightToe_End
          { offset -0.0842401 0.048543 1.36778
            channel Quat
          }
        }
      }
    }
  }
}

end
