#include "vis_Window.hpp"

#include <random>

namespace vis{
  vis::Window::PerformanceTest::PerformanceTest(vis::Window& window) : mWindow(window) {}

  void Window::PerformanceTest::start() {
    mStart = std::chrono::high_resolution_clock::now();
    mLast = std::chrono::high_resolution_clock::now();
    mFrameCount = 0;
    mMinimumMsPerFrame = 10000.0;
    mMaximumMsPerFrame = 0.0;
    mInitRun = true;
    mTimes = std::vector(mSteps, 0.0);
    mIsRunning = true;
  }
  const bool& Window::PerformanceTest::isRunning() const {return mIsRunning;}
/*
  void Window::PerformanceTest::executeIf(const bool& execute) {
    if(not execute) return;
    auto current = std::chrono::high_resolution_clock::now();
    if(not mInitRun){
      double frameTimeInSeconds = std::chrono::duration_cast<std::chrono::duration<double>>(current - mLast).count();
      mMinimumMsPerFrame = std::min(mMinimumMsPerFrame, 1000*frameTimeInSeconds);
      mMaximumMsPerFrame = std::max(mMaximumMsPerFrame, 1000*frameTimeInSeconds);
    } else mInitRun = false;
    mLast = current;
    if(++mFrameCount > mSteps) {
      double elapsed_seconds = std::chrono::duration_cast<std::chrono::duration<double>>(mLast - mStart).count();
      //std::cout << "elapsed_seconds:" << elapsed_seconds << std::endl;
      //std::cout << "minimum_ms_per_frame: " << mMinimumMsPerFrame << std::endl;
      //std::cout << "average_ms_per_frame: " << 1000*elapsed_seconds / static_cast<double>(mSteps) << std::endl;
      //std::cout << "maximum_ms_per_frame: " << mMaximumMsPerFrame << std::endl;
      std::cout << "mMinimumMsPerFrame & avg0[s] & mMaximumMsPerFrame = " << mMinimumMsPerFrame << " & " << 1000*elapsed_seconds / static_cast<double>(mSteps) << " & " << mMaximumMsPerFrame << std::endl;
      mIsRunning = false;
      return;
    }
    switch(mWindow.mMode) {
      case prism: mWindow.mCamera->moveOnSphere(-2*360.0f/mSteps, 0); break;
      case rayCast: mWindow.mCameraAlt->moveOnSphere(-2*360.0f/mSteps, 0); break;
    }
  }*/
  void Window::PerformanceTest::executeIf(const bool& execute) {
    if(not execute) return;
    auto current = std::chrono::high_resolution_clock::now();
    if(mInitRun){
      mInitRun = false;
    } else {
      mTimes[mFrameCount] = 1000.0*std::chrono::duration_cast<std::chrono::duration<double>>(current - mLast).count();
      mFrameCount++;
    }
    if(mFrameCount == mTimes.size()) {
      double avg = 0.0;
      for(const auto& time : mTimes) avg+= time/mSteps;
      //avg = avg/mSteps;
      double std = 0.0;
      for(const auto& time : mTimes) std+= (time-avg)*(time-avg)/mSteps;
      std = std::sqrt(std);
      std::cout << "avg[s] & std[s] = " << avg << " & " << std << std::endl;
      mIsRunning = false;
      return;
    }
    mLast = current;
    switch(mWindow.mMode) {
      case prism: mWindow.mCamera->moveOnSphere(-2*360.0f/mSteps, 0); break;
      case rayCast: mWindow.mCameraAlt->moveOnSphere(-2*360.0f/mSteps, 0); break;
    }
  }
;
  vis::Window::LoadingTest::LoadingTest(vis::Window& window) : mWindow(window) {}

  void Window::LoadingTest::start() {
    mIsRunning = true;
    mFrameCount = 0;
    mInitRun = true;
    mTimes = std::vector(mRuns*mDataSets.size()*mTasks, 0.0);
  }
  const bool& Window::LoadingTest::isRunning() const {return mIsRunning;}

  void Window::LoadingTest::executeIf(const bool& execute) {
    if(not execute) return;
    auto current = std::chrono::high_resolution_clock::now();
    if(mInitRun){
      mInitRun = false;
    } else {
      mTimes[mFrameCount] = std::chrono::duration_cast<std::chrono::duration<double>>(current - mLast).count();
      mFrameCount++;
    }
    if(mFrameCount == mTimes.size()) {
      for(int set = 0; set < mDataSets.size(); ++set) {
        for(int task = 0; task < mTasks; ++task) {
          double avg = 0.0;
          for(int run = 0; run < mRuns; ++run) {
            avg+= mTimes[run*mDataSets.size()*mTasks + set*mTasks + task];
          }
          avg = avg/mRuns;
          double std = 0.0;
          for(int run = 0; run < mRuns; ++run) {
            double diff = mTimes[run*mDataSets.size()*mTasks + set*mTasks + task]-avg;
            std+= diff*diff;
          }
          std = std::sqrt(std/mRuns);
          std::cout << "avg[s] & std[s] = " << avg << " & " << std << std::endl;
        }

      }
      //for(int i = 0; i < mTimes.size(); ++i) {
      //  std::cout << "mTimes[" << i << "]:" << mTimes[i] << std::endl;
      //}
      mIsRunning = false;
      return;
    }
    mLast = current;
    if(mFrameCount%mTasks == 0) {
      mWindow.loadContext(mDataSets[(mFrameCount/mTasks)%mDataSets.size()]);
    }
    if(mFrameCount%mTasks == 1) {
      std::mt19937 rng(mFrameCount);
      std::uniform_int_distribution<std::mt19937::result_type> distComp0(0, mWindow.mContext->numComponents(0)-1);
      std::uniform_int_distribution<std::mt19937::result_type> distSets0(0, mWindow.mContext->numComponentsSets(0)-1);
      mWindow.enterNextStage(distSets0(rng), distComp0(rng));
    }
    if(mFrameCount%mTasks == 2) {
      std::mt19937 rng(mFrameCount);
      std::uniform_int_distribution<std::mt19937::result_type> distComp1(0, mWindow.mContext->numComponents(1)-1);
      std::uniform_int_distribution<std::mt19937::result_type> distSets1(0, mWindow.mContext->numComponentsSets(1)-1);
      mWindow.enterNextStage(distSets1(rng), distComp1(rng));
    }
    if(mFrameCount%mTasks == 3) {
      std::mt19937 rng(mFrameCount);
      std::uniform_int_distribution<std::mt19937::result_type> distComp2(0, mWindow.mContext->numComponents(2)-1);
      std::uniform_int_distribution<std::mt19937::result_type> distSets2(0, mWindow.mContext->numComponentsSets(2)-1);
      mWindow.enterNextStage(distSets2(rng), distComp2(rng));
    }
  }
  void Window::PerformanceTest::executeTest2() {
    std::vector<std::string> dataSets = {"../data/simplex(dim=6_points=10000)"};//,
                                         //"../data/simplex(dim=12_points=10000)",
                                         //"../data/simplex(dim=18_points=10000)"};
    std::vector<double> min0(dataSets.size(), 10000.0);
    std::vector<double> max0(dataSets.size(), 0.0);
    std::vector<double> sum0(dataSets.size(), 0.0);
    std::vector<double> min1(dataSets.size(), 10000.0);
    std::vector<double> max1(dataSets.size(), 0.0);
    std::vector<double> sum1(dataSets.size(), 0.0);
    std::vector<double> min2(dataSets.size(), 10000.0);
    std::vector<double> max2(dataSets.size(), 0.0);
    std::vector<double> sum2(dataSets.size(), 0.0);
    std::vector<double> min3(dataSets.size(), 10000.0);
    std::vector<double> max3(dataSets.size(), 0.0);
    std::vector<double> sum3(dataSets.size(), 0.0);

    int runs = 5;
    for(int run = 0; run < runs; ++run) {
      for(int dataSet = 0; dataSet < dataSets.size(); ++dataSet) {
        auto start = std::chrono::high_resolution_clock::now();
        mWindow.loadContext(dataSets[dataSet]);
        auto end = std::chrono::high_resolution_clock::now();
        double timeInSeconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();;
        min0[dataSet] = std::min(min0[dataSet], timeInSeconds);
        max0[dataSet] = std::max(max0[dataSet], timeInSeconds);
        sum0[dataSet]+= timeInSeconds;
        std::cout << "min0[s] & avg0[s] & max0[s] = " << min0[dataSet] << " & " << sum0[dataSet]/runs << " & " << max0[dataSet] << std::endl;

        std::random_device dev;
        std::mt19937 rng(run);
        std::uniform_int_distribution<std::mt19937::result_type> distComp0(0, mWindow.mContext->numComponents(0)-1);
        std::uniform_int_distribution<std::mt19937::result_type> distSets0(0, mWindow.mContext->numComponentsSets(0)-1);
        start = std::chrono::high_resolution_clock::now();
        mWindow.enterNextStage(distSets0(rng), distComp0(rng));
        end = std::chrono::high_resolution_clock::now();
        timeInSeconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();;
        min1[dataSet] = std::min(min1[dataSet], timeInSeconds);
        max1[dataSet] = std::max(max1[dataSet], timeInSeconds);
        sum1[dataSet]+= timeInSeconds;
        std::cout << "min1[s] & avg1[s] & max1[s] = " << min1[dataSet] << " & " << sum1[dataSet]/runs << " & " << max1[dataSet] << std::endl;

        std::uniform_int_distribution<std::mt19937::result_type> distComp1(0, mWindow.mContext->numComponents(1)-1);
        std::uniform_int_distribution<std::mt19937::result_type> distSets1(0, mWindow.mContext->numComponentsSets(1)-1);
        start = std::chrono::high_resolution_clock::now();
        mWindow.enterNextStage(distSets1(rng), distComp1(rng));
        end = std::chrono::high_resolution_clock::now();
        timeInSeconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();;
        min2[dataSet] = std::min(min2[dataSet], timeInSeconds);
        max2[dataSet] = std::max(max2[dataSet], timeInSeconds);
        sum2[dataSet]+= timeInSeconds;
        std::cout << "min2[s] & avg2[s] & max2[s] = " << min2[dataSet] << " & " << sum2[dataSet]/runs << " & " << max2[dataSet] << std::endl;

        std::uniform_int_distribution<std::mt19937::result_type> distComp2(0, mWindow.mContext->numComponents(2)-1);
        std::uniform_int_distribution<std::mt19937::result_type> distSets2(0, mWindow.mContext->numComponentsSets(2)-1);
        start = std::chrono::high_resolution_clock::now();
        mWindow.enterNextStage(distSets2(rng), distComp2(rng));
        end = std::chrono::high_resolution_clock::now();
        timeInSeconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();;
        min3[dataSet] = std::min(min3[dataSet], timeInSeconds);
        max3[dataSet] = std::max(max3[dataSet], timeInSeconds);
        sum3[dataSet]+= timeInSeconds;
        std::cout << "min3[s] & avg3[s] & max3[s] = " << min3[dataSet] << " & " << sum3[dataSet]/runs << " & " << max3[dataSet] << std::endl;
      }
    }
    for(int dataSet = 0; dataSet < dataSets.size(); ++dataSet) {
      std::cout << "[Info](Window::PerformanceTest::executeTest2) " << dataSets[dataSet] << std::endl;
      std::cout << "min0[s] & avg0[s] & max0[s] = " << min0[dataSet] << " & " << sum0[dataSet]/runs << " & " << max0[dataSet] << std::endl;
      std::cout << "min1[s] & avg1[s] & max1[s] = " << min1[dataSet] << " & " << sum1[dataSet]/runs << " & " << max1[dataSet] << std::endl;
      std::cout << "min2[s] & avg2[s] & max2[s] = " << min2[dataSet] << " & " << sum2[dataSet]/runs << " & " << max2[dataSet] << std::endl;
      std::cout << "min3[s] & avg3[s] & max3[s] = " << min3[dataSet] << " & " << sum3[dataSet]/runs << " & " << max3[dataSet] << std::endl;
    }
  }
} // namespace vis
