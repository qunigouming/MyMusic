#include "SongListPage.h"
#include <QJsonObject>
#include <QJsonDocument>

#include "LogManager.h"
#include "tcpmanager.h"
#include "UserManager.h"
#include "NetworkImageLabel.h"

SongListPage::SongListPage(QWidget *parent)
	: QWidget(parent)
{
	_mainLayout = new QVBoxLayout();

	_topLayout = new QHBoxLayout();
	_songlist_icon = new NetworkImageLabel();
	_songlist_icon->setFixedSize(200, 200);
	_topLayout->addWidget(_songlist_icon);

	_songlist_info_layout = new QVBoxLayout();

	_songlist_info_top_layout = new QHBoxLayout();
	_songlist_name_lab = new QLabel(tr("歌单名称"));
	_songlist_name_lab->setStyleSheet("font-size: 20px; font-weight: 700;");
	_songlist_info_top_layout->addWidget(_songlist_name_lab);

	_songlist_info_layout->addLayout(_songlist_info_top_layout);

	_songlist_info_desc_layout = new QHBoxLayout();
	_songlist_desc_lab = new QLabel(tr("歌单描述信息"));
	_songlist_desc_lab->setStyleSheet("font-size: 14px;");
	_songlist_desc_lab->setWordWrap(true);

	_songlist_info_layout->addLayout(_songlist_info_desc_layout);

	_songlist_info_middle_layout = new QHBoxLayout();
	_songlist_author_icon_lab = new QLabel();

	_songlist_author_icon_lab->setPixmap(QPixmap().scaled(20, 20));
	_songlist_author_lab = new QLabel(tr("作者名称"));
	_songlist_author_lab->setStyleSheet("font-size: 14px;");
	_songlist_create_time_lab = new QLabel(tr("创建时间"));
	_songlist_create_time_lab->setStyleSheet("font-size: 14px;");

	_songlist_info_middle_layout->addWidget(_songlist_author_icon_lab);
	_songlist_info_middle_layout->addWidget(_songlist_author_lab);
	_songlist_info_middle_layout->addWidget(_songlist_create_time_lab);
	_songlist_info_middle_layout->addStretch();

	_songlist_info_layout->addLayout(_songlist_info_middle_layout);
	_songlist_info_layout->addStretch();

	_songlist_func_layout = new QHBoxLayout();
	_songlist_play_all_btn = new QPushButton("播放全部");
	_songlist_func_layout->addWidget(_songlist_play_all_btn);

	_songlist_download_btn = new QPushButton("下载");
	_songlist_func_layout->addWidget(_songlist_download_btn);
	_songlist_share_btn = new QPushButton("分享");
	_songlist_func_layout->addWidget(_songlist_share_btn);
	_songlist_func_layout->addStretch();

	_songlist_info_layout->addLayout(_songlist_func_layout);

	_topLayout->addLayout(_songlist_info_layout);

	_mainLayout->addLayout(_topLayout);

	// 处理Tab窗口
	_tabWidget = new QTabWidget();

	connect(_tabWidget, &QTabWidget::currentChanged, [this](int index) {
		LOG(INFO) << "current Change";
		if (!_pageInfo) return;
		if (index == 0) {
			// 发送请求歌单歌曲列表消息
			QJsonObject req;
			req["playlist_name"] = _pageInfo->title;
			req["fromuid"] = UserManager::GetInstance()->getUid();
			QJsonDocument reqDoc(req);
			emit TcpManager::GetInstance()->slot_send_data(ReqID::ID_GET_COLLECT_SONG_LIST_REQ, reqDoc.toJson());
		}
	});

	_tableView = new TableView(MusicTableViewType::NET_MODEL);
	// 添加页签
	_tabWidget->addTab(_tableView, tr("歌曲"));
	_tabWidget->addTab(new QWidget, tr("评论"));
	//_tabWidget->setCurrentIndex(1);
	_mainLayout->addWidget(_tabWidget);

	this->setLayout(_mainLayout);

	connect(TcpManager::GetInstance().get(), &TcpManager::sig_song_list_page_info, this, &SongListPage::setupUI);
	connect(TcpManager::GetInstance().get(), &TcpManager::sig_song_list_page_songs, this, [this](QList<std::shared_ptr<MusicInfo>> musiclist) {
		LOG(INFO) << "Get all song of songlist";
		_tableView->clearAllSongs();
		for (auto music : musiclist) {
            _tableView->addSong(SongInfo(music.get()));
		}
	});
}

SongListPage::~SongListPage()
{
}

void SongListPage::setupUI(std::shared_ptr<SongListPageInfo> info)
{
    _pageInfo = info;
	emit _tabWidget->currentChanged(0);
	if (_pageInfo->songlist_icon.isEmpty()) {
		_songlist_icon->setPixmap(QPixmap(":/source/image/default_album.png").scaled(200, 200));
	}
	else {
		_songlist_icon->setImageUrl(_pageInfo->songlist_icon);
	}

	_songlist_name_lab->setText(_pageInfo->title);
	if (_pageInfo->isModify) {
		_songlist_name_modify_btn = new QPushButton();
        _songlist_name_modify_btn->setObjectName("songlist_name_modify_btn");
        _songlist_name_modify_btn->setText(QChar(0xe100));
        _songlist_name_modify_btn->setStyleSheet(R"(
			#songlist_name_modify_btn {
				border: none;
				font-family: "otherfont";
				font-size: 18px;
				color: rgb(100, 100, 100);
			}

			#songlist_name_modify_btn:hover {
				color: rgb(44,  44, 44);
			}
		)");
        _songlist_info_top_layout->addWidget(_songlist_name_modify_btn);
	}
	_songlist_desc_lab->setText(_pageInfo->description);
	
    _songlist_author_icon_lab->setPixmap(QPixmap(_pageInfo->authorIcon).scaled(20, 20));
	_songlist_author_lab->setText(_pageInfo->author);
	_songlist_create_time_lab->setText(_pageInfo->createTime + tr("创建"));

	if (_pageInfo->isCollect) {
		_songlist_collect_btn = new QPushButton("收藏");
		_songlist_func_layout->addWidget(_songlist_collect_btn);
	}
}

