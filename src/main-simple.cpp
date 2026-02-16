/**
 * @file main-simple.cpp
 * @brief 간단한 webOS Native Browser
 * 
 * Qt 없이 순수 C++로 구현
 * webOS WebView API 사용
 */

#include <iostream>
#include <cstdlib>
#include <unistd.h>

extern "C" {
    // webOS API 스텁 (실제 webOS에서는 런타임이 제공)
    void* WAM_createWebView(const char* appId) {
        std::cout << "[WAM_createWebView] appId=" << appId << std::endl;
        return (void*)0x1; // 더미 포인터
    }

    void WAM_loadURL(void* webView, const char* url) {
        std::cout << "[WAM_loadURL] webView=" << webView << ", url=" << url << std::endl;
    }

    void WAM_destroy(void* webView) {
        std::cout << "[WAM_destroy] webView=" << webView << std::endl;
    }
}

int main(int argc, char* argv[]) {
    std::cout << "webOS Native Browser Starting..." << std::endl;
    
    // 기본 URL
    const char* defaultUrl = "https://www.google.com";
    const char* url = (argc > 1) ? argv[1] : defaultUrl;
    
    std::cout << "Loading URL: " << url << std::endl;
    
    // webOS WebView 생성 (WAM 사용)
    void* webView = WAM_createWebView("com.jsong.webosbrowser.native");
    
    if (webView) {
        std::cout << "WebView created successfully" << std::endl;
        
        // URL 로드
        WAM_loadURL(webView, url);
        
        std::cout << "URL loaded, entering event loop..." << std::endl;
        
        // 이벤트 루프 (실제로는 webOS 런타임이 처리)
        // 여기서는 단순히 대기
        std::cout << "Press Ctrl+C to exit" << std::endl;
        while (true) {
            // 이벤트 처리는 webOS 런타임이 담당
            sleep(1);
        }
        
        // 정리
        WAM_destroy(webView);
    } else {
        std::cerr << "Failed to create WebView" << std::endl;
        return 1;
    }
    
    return 0;
}
