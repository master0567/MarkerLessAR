#include "Asift.h"


Asift::Asift(void)
{
}


Asift::~Asift(void)
{
}

void Asift::init(int readini,string img1_name,string img2_name,
			string imgV_name,string imgH_name,
			string keys1_name,string keys2_name,
			string matchings_name,string capIn_name,string capOut_name,
			int tilt1,int tilt2,int resize_flag){

	// 設定初期化

	img1Name=img1_name;
	img2Name=img2_name;
	imgVName=imgV_name;
	imgHName=imgH_name;
	keys1Name = keys1_name;
	keys2Name = keys2_name;
	matchingsName = matchings_name;
	capInName = capIn_name;
	capOutName = capOut_name;
	resizeFlag = resize_flag;

	keys1 = AsiftKeypoints(tilt1);
	keys2 = AsiftKeypoints(tilt2);

	verb = 0;
	bandWidth = 20;

	keys1InputFlag = 0;
	videoInputFlag = 0;

	// iniファイルから読み込み
	if(readini){
	
		readIni();
	
	}

	// 画像読み込み
	//if(!keys1InputFlag){
		img1.load(img1Name);
		img1Gray.grayeScale(img1);
		resize(img1Gray,img1GrayZoom,zoom1,resizeFlag);
		unsigned char * iarr1 = img1GrayZoom.getU8Data();
		ipixels1Zoom = std::vector<float>(iarr1,iarr1 + img1GrayZoom.size().width * img1GrayZoom.size().height);
	//}
	if(!videoInputFlag){
		img2.load(img2Name);
		img2Gray.grayeScale(img2);
		resize(img2Gray,img2GrayZoom,zoom2,resizeFlag);
		unsigned char * iarr2 = img2GrayZoom.getU8Data();
		ipixels2Zoom = std::vector<float>(iarr2,iarr2 + img2GrayZoom.size().width * img2GrayZoom.size().height);
	}


}

void Asift::writeIni(){
	ptree pt;

	// write path
	pt.put("File.InputImage1",img1Name);
	pt.put("File.InputImage2",img2Name);
	pt.put("File.OutPutVertImage",imgVName);
	pt.put("File.OutputHoriImage",imgHName);
	pt.put("File.OutputKeyPoints1",keys1Name);
	pt.put("File.OutputKeyPoints2",keys2Name);
	pt.put("File.OutputMatching",matchingsName);
	pt.put("File.CapInput",capInName);
	pt.put("File.CapOutput",capOutName);

		capInName = pt.get_optional<std::string>("File.CapInput").get();
		capOutName = pt.get_optional<std::string>("File.CapOutput").get();
	
	// write Asift parameter
	pt.put("Asift.Tilt1",keys1.getTilts());
	pt.put("Asift.Tilt2",keys2.getTilts());
	pt.put("Asift.ResizeFlag",resizeFlag);
	pt.put("Asift.Verb",verb);
	pt.put("Asift.bandWidth",bandWidth);

	// write Options 
	pt.put("Option.keys1FileInput",keys1InputFlag);
	pt.put("Option.videoInput",videoInputFlag);

	// write Sift Parameter
	keys1.iniwriteSiftParameters(pt);

	// writing
	write_ini("Asift.ini",pt);
}

void Asift::readIni(){


	if(path.isExist("Asift.ini")){

		ptree pt;
		read_ini("Asift.ini", pt);

		// read path
		img1Name = pt.get_optional<std::string>("File.InputImage1").get();
		img2Name = pt.get_optional<std::string>("File.InputImage2").get();
		imgVName = pt.get_optional<std::string>("File.OutPutVertImage").get();
		imgHName = pt.get_optional<std::string>("File.OutputHoriImage").get();
		keys1Name = pt.get_optional<std::string>("File.OutputKeyPoints1").get();
		keys2Name = pt.get_optional<std::string>("File.OutputKeyPoints2").get();
		matchingsName = pt.get_optional<std::string>("File.OutputMatching").get();
		capInName = pt.get_optional<std::string>("File.CapInput").get();
		capOutName = pt.get_optional<std::string>("File.CapOutput").get();

		// read Asift Parameter
		int tilts1 = pt.get_optional<int>("Asift.Tilt1").get();
		int tilts2 = pt.get_optional<int>("Asift.Tilt2").get();

		keys1 = AsiftKeypoints(tilts1);
		keys2 = AsiftKeypoints(tilts2);
		
		// read Asift Parameter options
		resizeFlag = pt.get_optional<int>("Asift.ResizeFlag").get();
		verb = pt.get_optional<int>("Asift.Verb").get();
		bandWidth = pt.get_optional<int>("Asift.bandWidth").get();

		// read Options
		keys1InputFlag = pt.get_optional<int>("Option.keys1FileInput").get();
		videoInputFlag = pt.get_optional<int>("Option.videoInput").get();

		// Read Sift Parameter 
		keys1.inireadSiftParameters(pt);
		keys2.inireadSiftParameters(pt);

	}else{

		std::cout << "not exist Asift.ini" << endl;
		writeIni();

	}

}


void Asift::run(){
	time_t tstart, tend;
	pro::Timer timer;
	
	/***********************************************************
	 *  Keypoints1 
	 */
	std::cout << "Computing keypoints1 on the two images..." << endl;
	timer.start();

		if(!keys1InputFlag){
			keys1.computeAsiftKeyPoints(ipixels1Zoom,img1GrayZoom.size().width,img1GrayZoom.size().height,verb,zoom1);
		}else{
			keys1.input(keys1Name);
		}
		
			
	std::cout << keys1.getNum() <<  " ASIFT keypoints are detected." << endl;
	std::cout << "Keypoints1 computation accomplished in " << (double)timer.getDiff()/pro::Timer::PER_SEC << " seconds." << endl << endl;
	timer.lap();

	/**
	 * 動画読み込み処理
	 */
	if(videoInputFlag){

		// 読み込み動画
		cv::VideoCapture cap(capInName);
		// ファイルがオープンできたかの確認
		if(!cap.isOpened()) return;

		// ビデオライタ
		int fps = 15;
		cv::VideoWriter writer(capOutName, CV_FOURCC('X','V','I','D'), fps, cv::Size(800*2+20,600));
	
		pro::Image frame;

		// 進行状況
		int count = 0;
		int framenum = cap.get(CV_CAP_PROP_FRAME_COUNT);

		// 表示
		std::cout << "video Input is " << capInName << "." << endl;
		std::cout << "video Output is " << capOutName << "." << endl;
		cout << "start" << endl;

		while(1) {
			cap.read((cv::Mat&)frame);  // キャプチャ

			if(frame.empty()){
				break;
			}

			img2Gray.grayeScale(frame);
			resize(img2Gray,img2GrayZoom,zoom2,0);
			unsigned char * iarr2 = img2Gray.getU8Data();
			ipixels2Zoom = std::vector<float>(iarr2,iarr2 + img2GrayZoom.size().width * img2GrayZoom.size().height);
	
			std::cout << ++count << "/" << framenum << "\r" << flush;
			keys2.computeAsiftKeyPoints(ipixels2Zoom,img2GrayZoom.size().width,img2GrayZoom.size().height,verb,zoom2);
			matchings = AsiftMatchings(keys1,keys2);
			matchings.computeAsiftMatches(verb);

			// 水平画像作成
			writer << createHoriImage(frame);

			//writer << frame;

			if(cv::waitKey(30) >= 0) break;

			delete[] iarr2;
		}

	/**
	 * 画像読み込み処理
	 */
	}else{

		/***********************************************************
		 *  Keypoints2
		 */
		std::cout << "Computing keypoints2 on the two images..." << endl;

			keys2.computeAsiftKeyPoints(ipixels2Zoom,img2GrayZoom.size().width,img2GrayZoom.size().height,verb,zoom2);
			std::cout << keys2.getNum() <<  " ASIFT keypoints are detected." << endl;

		std::cout << "Keypoints2 computation accomplished in " << (double)timer.getDiff()/pro::Timer::PER_SEC << " seconds." << endl << endl;
		timer.lap();
	
		/***********************************************************
		 *  Keypoints matching
		 */
		std::cout << "Matching the keypoints..." << endl;

			matchings = AsiftMatchings(keys1,keys2);
			matchings.computeAsiftMatches(verb);
		
		std::cout << "The two images match! " << matchings.getNum() << " matchings are identified." << endl;
		std::cout << "Keypoints matching accomplished in " << (double)timer.getDiff()/pro::Timer::PER_SEC << " seconds." << endl << endl;
		timer.lap();

		std::cout << "All computation accomplished in " << (double)timer.getNow()/pro::Timer::PER_SEC << "seconds." << endl << endl;
		
		// 水平画像作成
		createHoriImage(img2);

		// 水平画像作成
		createVertImage(img2);

		// キーポイントファイル作成
		keys1.output(keys1Name);
		keys2.output(keys2Name);
	
		// マッチングファイル作成
		matchings.output(matchingsName);
	
	}

}

void Asift::resize(pro::Image &src,pro::Image &dst,float &zoom,int resizeFlag){
	if(!resizeFlag){
		dst.clone(src);
		zoom = 1;
	}else{
		//std::cout << "WARNING: The input images are resized to " << IMAGE_X << "x" << IMAGE_Y << " for ASIFT. " << endl 
		//<< "         But the results will be normalized to the original image size." << endl << endl;
		
		float InitSigma_aa = 1.6;
		
		int wS,hS,w,h;
				
		float areaS = IMAGE_X * IMAGE_Y;

		w = src.size().width;
		h = src.size().height;

		float area1 = w * h;
		zoom = sqrt(area1/areaS);
		
		wS = (int) (w / zoom);
		hS = (int) (h / zoom);
		
		/* Anti-aliasing filtering along vertical direction */
		if ( zoom > 1 )
		{
			int ksize;
			float sigma_aa = InitSigma_aa * zoom / 2;
			const float GaussTruncate1 = 4.0;
			ksize = (int)(2.0 * GaussTruncate1 * sigma_aa + 1.0);
			ksize = MAX(3, ksize);    /* Kernel must be at least 3. */
			if (ksize % 2 == 0)       /* Make kernel size odd. */
				ksize++;
			assert(ksize < 100);
			cv::GaussianBlur((const cv::Mat&)src, (cv::Mat&)dst, cv::Size(ksize,ksize), sigma_aa, sigma_aa);
		}
			
		dst.resize(src,cv::Size(wS,hS));
		
	}
}

pro::Image Asift::createVertImage(pro::Image img){
	pro::Image vertImg;
	vertImg.vertconcat(img1,img,bandWidth);
	matchingslist::iterator ptr = matchings.matchings.begin();
	for(int i=0; i < (int) matchings.matchings.size(); i++, ptr++)
	{		
		vertImg.line(cv::Point2f((zoom1*ptr->first.x),(zoom1*ptr->first.y)), 
			cv::Point2f((zoom2*ptr->second.x),(zoom2*ptr->second.y) + img1.size().height + bandWidth),cv::Scalar::all(255));
	}
	if(!videoInputFlag)
		vertImg.save(imgVName);
	return vertImg;
}

pro::Image Asift::createHoriImage(pro::Image img){
	pro::Image horiImg;
	horiImg.horiconcat(img1,img,bandWidth);
	matchingslist::iterator ptr = matchings.matchings.begin();
	for(int i=0; i < (int) matchings.matchings.size(); i++, ptr++)
	{		
		horiImg.line(cv::Point2f((zoom1*ptr->first.x),(zoom1*ptr->first.y)), 
			cv::Point2f((zoom2*ptr->second.x) + img1.size().width + bandWidth,(zoom2*ptr->second.y)),cv::Scalar::all(255));
	}
	if(!videoInputFlag)
		horiImg.save(imgHName);
	return horiImg;
}