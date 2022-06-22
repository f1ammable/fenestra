#include <iostream>
#include <vector>
#include <string>
#include <Windows.h>

struct monitor_info {
	std::vector<int> i_monitors;
	std::vector<HMONITOR> h_monitors;
	std::vector<HDC> hdc_monitors;
	std::vector<RECT> rc_monitors;

	static BOOL CALLBACK monitor_enum(HMONITOR h_mon, HDC hdc, LPRECT lprc_mon, LPARAM p_data) {
		auto* p_this = reinterpret_cast<monitor_info*>(p_data);
		p_this->h_monitors.push_back(h_mon);
		p_this->hdc_monitors.push_back(hdc);
		p_this->rc_monitors.push_back(*lprc_mon);
		p_this->i_monitors.push_back(p_this->hdc_monitors.size());
		return TRUE;
	}

	monitor_info() {
		EnumDisplayMonitors(nullptr, nullptr, monitor_enum, LPARAM(this));
	}
} s_monitor_info;

struct window_info
{
	std::vector<HWND> window_handles;
	std::vector<std::string> window_titles;

	static BOOL CALLBACK window_enum(HWND hWnd, LPARAM lparam) {
		auto* p_this = reinterpret_cast<window_info*>(lparam);
		const long long int length = GetWindowTextLength(hWnd);
		auto* title_buf = new wchar_t[length + 1];
		GetWindowText(hWnd, (LPWSTR)title_buf, length + 1);
		std::wstring ws_window_title(title_buf);
		std::string window_title(ws_window_title.begin(), ws_window_title.end());


		if (IsWindowVisible(hWnd) && length != 0) {

			#ifdef _DEBUG
				std::cout << hWnd << " - " << window_title << std::endl;
			#endif

			p_this->window_titles.push_back(window_title);
			p_this->window_handles.push_back(hWnd);
		}

		delete[] title_buf;
		return TRUE;
	}

	window_info()
	{
		EnumWindows(window_enum, LPARAM(this));
	}

} s_window_info;


void move_app(HWND app_hwnd, monitor_info s_mon, int window_index)
{

	RECT app_rect;

	//Co-ordinates of chosen monitor
	const auto mon_left = &s_mon.rc_monitors[window_index].left;
	const auto mon_right = &s_mon.rc_monitors[window_index].right;
	const auto mon_top = &s_mon.rc_monitors[window_index].top;
	const auto mon_bottom = &s_mon.rc_monitors[window_index].bottom;

	//Calculate the middle rect for the specified monitor
	const auto mon_x_center = (*mon_left + *mon_right) / 2;
	const auto mon_y_center = (*mon_top + *mon_bottom) / 2;

	//Get centre of app window
	GetWindowRect(app_hwnd, &app_rect);

	//Calculate centre of app window
	const auto app_x_center = (app_rect.left + app_rect.right) / 2;
	const auto app_y_center = (app_rect.top + app_rect.bottom) / 2;

	MoveWindow(app_hwnd, mon_x_center, mon_y_center, app_rect.right - app_rect.left, app_rect.bottom - app_rect.top, TRUE);

	// Centre app window to centre of monitor, TODO: deal with negative monitor coords
	//SetWindowPos(app_hwnd, nullptr, app_x_center - mon_x_center, app_y_center - mon_y_center, 0, 0, SWP_NOZORDER | SWP_SHOWWINDOW | SWP_NOSIZE);
}

int main(void) {
	int window_input;

	std::cout << "\n\n\nWhich window do you want to move? Enter the number of the window listed below:\n\n";
	for (int i = 0; i < s_window_info.window_titles.size(); i++)
	{
		std::cout << "[" << i << "] " << s_window_info.window_titles[i] << std::endl;
	}

	std::cout << std::endl;
	std::cin >> window_input;


	// check for valid input
	if (window_input < 0 || window_input >=s_window_info.window_titles.size())
	{
		std::cout << "Invalid number." << std::endl;
	}
	else
	{
		int mon_input;
		#ifdef _DEBUG
		std::cout << "You entered '" << window_input << "', which corresponds to window '" << s_window_info.window_titles[window_input] << "' with handle: " << s_window_info.window_handles[window_input] << std::endl;
		#endif

		std::cout << "Please select the monitor you want to move window " << s_window_info.window_titles[window_input] << " to\n";

		for(const auto &m : s_monitor_info.i_monitors)
		{
			std::cout << m << std::endl;
		}

		std::cin >> mon_input;

		if (mon_input < 0 || mon_input > s_monitor_info.hdc_monitors.size())
		{
			std::cout << "Invalid monitor selected\n";
		}
		else
		{
			move_app(s_window_info.window_handles[window_input], s_monitor_info, mon_input - 1);
		}
	}
}
