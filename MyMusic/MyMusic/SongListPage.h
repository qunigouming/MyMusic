#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include "tableview/tableview.h"
#include <QTabWidget>
#include "dataInfo.h"

class SongListPage : public QWidget
{
	Q_OBJECT

public:
	SongListPage(QWidget *parent = nullptr);
	~SongListPage();

	void setupUI(std::shared_ptr<SongListPageInfo> info);

private:
	std::shared_ptr<SongListPageInfo> _pageInfo;

	// 布局相关
	QVBoxLayout* _mainLayout = nullptr;

	QHBoxLayout* _topLayout = nullptr;
	QLabel* _songlist_icon = nullptr;

	QVBoxLayout* _songlist_info_layout = nullptr;
	QHBoxLayout* _songlist_info_top_layout = nullptr;
    QLabel* _songlist_name_lab = nullptr;
	QPushButton* _songlist_name_modify_btn = nullptr;

	QHBoxLayout* _songlist_info_desc_layout = nullptr;
	QLabel* _songlist_desc_lab = nullptr;

    QHBoxLayout* _songlist_info_middle_layout = nullptr;
	QLabel* _songlist_author_icon_lab = nullptr;
	QLabel* _songlist_author_lab = nullptr;
    QLabel* _songlist_create_time_lab = nullptr;

    QHBoxLayout* _songlist_func_layout = nullptr;
	QPushButton* _songlist_play_all_btn = nullptr;
	QPushButton* _songlist_collect_btn = nullptr;		// 收藏按钮，只在未收藏的歌单界面显示
	QPushButton* _songlist_download_btn = nullptr;
	QPushButton* _songlist_share_btn = nullptr;

	QTabWidget* _tabWidget = nullptr;
	TableView* _tableView = nullptr;
};

