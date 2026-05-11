#include "inventorywindow.h"
#include <QVBoxLayout>
#include <QLabel>

InventoryWindow::InventoryWindow(GameState *state, QWidget *parent)
    : QWidget(parent), m_state(state)
{
    setWindowTitle("背包");
    QVBoxLayout *layout = new QVBoxLayout(this);
    m_listWidget = new QListWidget(this);
    layout->addWidget(new QLabel("拥有的物品："));
    layout->addWidget(m_listWidget);

    connect(m_state, &GameState::inventoryChanged, this, &InventoryWindow::refreshList);
    refreshList();
}

void InventoryWindow::refreshList()
{
    m_listWidget->clear();
    for (Item item : m_state->inventoryItems()) {
        QString name;
        switch (item) {
        case Item::StudentCard: name = "学子卡"; break;
        case Item::AudioRecording: name = "一段钢琴录音"; break;
        case Item::WishCard: name = "心愿卡"; break;
        case Item::AuntHandkerchief: name = "阿姨的手帕"; break;
        case Item::OldAssignment: name = "陈旧的作业纸"; break;
        case Item::SciFiBookExcerpt: name = "科幻小说书摘"; break;
        case Item::PearCandy: name = "一小袋梨膏糖"; break;
        case Item::TornNote: name = "一张撕下来的笔记"; break;
        case Item::HalfChalk: name = "半截粉笔"; break;
        case Item::OneYuanCoin: name = "一枚一元硬币"; break;
        case Item::MapleBookmark: name = "枫叶书签"; break;
        case Item::OldDiaryPage: name = "泛黄的日记"; break;
        case Item::HandDrawnMap: name = "手绘地图"; break;
        case Item::InstantCoffee: name = "速溶咖啡"; break;
        case Item::LargeSycamoreLeaf: name = "很大的梧桐叶"; break;
        case Item::DarkBlueButton: name = "深蓝色纽扣"; break;
        case Item::KeychainWithBell: name = "钥匙扣（带铃铛）"; break;
        case Item::SportsDrink: name = "运动饮料"; break;
        case Item::Pinecone: name = "一颗松果"; break;
        case Item::DoodlePaper: name = "涂鸦草稿纸"; break;
        case Item::BrokenEarphone: name = "断掉的耳机线"; break;
        case Item::SmoothPebble: name = "圆润的鹅卵石"; break;
        default: name = "未知物品"; break;
        }
        m_listWidget->addItem(name);
    }
}