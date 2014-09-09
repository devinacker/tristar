/*
    This code is released under the terms of the MIT license.
    See COPYING.txt for details.
*/

#include <QPixmap>
#include <QPainter>
#include <QTimer>
#include <QFontMetrics>
#include <QGraphicsView>
#include <algorithm>
#include <stdexcept>
#include <cstdlib>
#include <list>
#include "level.h"
#include "mainwindow.h"
#include "mapscene.h"

#define MAP_TEXT_PAD_H 4
#define MAP_TEXT_PAD_V 0

// move to a graphics-related source file eventually idk
#define TILE_SIZE 16

const QFont MapScene::infoFont("Segoe UI", 10, QFont::Bold);
const QFontMetrics MapScene::infoFontMetrics(MapScene::infoFont);

const QColor MapScene::enemyColor(255, 255, 255, 192);
const QColor MapScene::objectColor(255, 192, 192, 192);
const QColor MapScene::itemColor(0, 192, 224, 192);

const QColor MapScene::infoBackColor(255, 192, 192, 128);

const QColor MapScene::selectionColor(255, 192, 192, 192);
const QColor MapScene::selectionBorder(255, 192, 192, 255);

/*
  Overridden constructor which inits some scene info
 */
MapScene::MapScene(QObject *parent, LevelData *currentLevel)
    : QGraphicsScene(parent),

      tileX(-1), tileY(-1),
      selLength(0), selWidth(0), selecting(false),
      stack(this),
      level(currentLevel),
      tilesetPixmap(256*TILE_SIZE, TILE_SIZE),
      animFrame(0), animTimer(this),
      showCollision(true),
      showVisual({true, true, true}),
      showBreakable(true),
      showObjects(true),
      showItems(true),
      showEnemies(true)
{
    QObject::connect(this, SIGNAL(edited()),
                     this, SLOT(update()));
    QObject::connect(&animTimer, SIGNAL(timeout()),
                     this, SLOT(animate()));
}

/*
  Redraw the scene
*/
void MapScene::refresh() {
    tileX = -1;
    tileY = -1;
    updateSelection();

    // reset the scene
    clear();

    // if level is null , minimize the scene and return
    if (!level) {
        setSceneRect(0, 0, 0, 0);
        return;
    }

    // TODO: set dimensions
    uint width = level->width;
    uint height = level->height;
    setSceneRect(0, 0, width * TILE_SIZE, height * TILE_SIZE);

    //setAnimSpeed(level->header.animSpeed);
    refreshPixmap();

    update();
}

void MapScene::setAnimSpeed(int speed) {
    // set up tile animation
    // frame length (NTSC frames -> msec)
    uint timeout = speed * 16;
    if (timeout) {
        animTimer.start(timeout);
    } else {
        animFrame = 0;
        animTimer.stop();
        refreshPixmap();
    }
}

const QPixmap* MapScene::getPixmap() const {
    return &this->tilesetPixmap;
}

void MapScene::refreshPixmap() {
    // ...
}

// advance to next animation frame
void MapScene::animate() {
    // ...
}

/*
  Handle when the mouse is pressed on the scene
*/
void MapScene::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    // if the level is not being displayed, don't do anything
    // (or if the click is outside of the scene)
    if (!isActive() || !sceneRect().contains(event->scenePos())) return;

    // left button: start or continue selection
    // right button: cancel selection
    if (event->buttons() & Qt::LeftButton) {
        beginSelection(event);
        event->accept();

    } else if (event->buttons() & Qt::RightButton) {
        cancelSelection();
        event->accept();
    }
    update();
}

/*
  Handle when a double-click occurs (used to start the tile edit window)
*/
void MapScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) {
    if (!isActive()) return;

    emit doubleClicked();
    event->accept();

    update();
}

/*
  Handle when the left mouse button is released
*/
void MapScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        selecting = false;

        // normalize selection dimensions (i.e. handle negative height/width)
        // so that selections made down and/or to the left are handled appropriately
        if (selWidth < 0) {
            selX += selWidth + 1;
            selWidth *= -1;
        }
        if (selLength < 0) {
            selY += selLength + 1;
            selLength *= -1;
        }

        event->accept();
    }
    update();
}

/*
  Handle when the mouse is moved over the scene
 */
void MapScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    // if inactive, don't handle mouse moves
    if (!isActive()) return;

    // behave differently based on left mouse button status
    if (selecting && event->buttons() & Qt::LeftButton) {
        // left button down: generate/show selection
        updateSelection(event);
    } else {
        showTileInfo(event);
    }

    event->accept();

    update();
}

/*
 *Undo/redo functions
 */
bool MapScene::canUndo() const {
    return stack.canUndo();
}

bool MapScene::canRedo() const {
    return stack.canRedo();
}

bool MapScene::isClean() const {
    return stack.isClean();
}

void MapScene::pushChange(QUndoCommand *change) {
    stack.push(change);
    emit edited();
}

void MapScene::undo() {
    if (stack.canUndo()) {
        emit statusMessage(QString("Undoing ").append(stack.undoText()));
        stack.undo();
        emit edited();

        // TODO: mark level modified
    }
}

void MapScene::redo() {
    if (stack.canRedo()) {
        emit statusMessage(QString("Redoing ").append(stack.redoText()));
        stack.redo();
        emit edited();

        // TODO: mark level modified
    }
}

void MapScene::setClean() {
    stack.setClean();
}

void MapScene::clearStack() {
    stack.clear();
}

/*
  Cut/copy/paste functions
*/
void MapScene::cut() {
    copyTiles(true);
}

void MapScene::copy() {
    copyTiles(false);
}

void MapScene::copyTiles(bool cut = false) {
    // ...
}

void MapScene::paste() {
    // ...
}

void MapScene::deleteStuff() {
    deleteTiles();
}

void MapScene::deleteTiles() {
    // ...
}

/*
 *remove scene items
 */
void MapScene::deleteItems() {
    QList<QGraphicsItem*> items = this->selectedItems();

    // iterate and delete
    // ...
    emit edited();
}

/*
  Start a new selection on the map scene.
  Called when the mouse is clicked outside of any current selection.
*/
void MapScene::beginSelection(QGraphicsSceneMouseEvent *event) {
/*
    QPointF pos = event->scenePos();

    int x = pos.x() / TILE_SIZE;
    int y = pos.y() / TILE_SIZE;

    // ignore invalid click positions
    // (use the floating point X coord to avoid roundoff stupidness)
    if (x >= level->width || y >= level->height || pos.x() < 0 || y < 0)
        return;

    // is the click position outside of the current selection?
    if (x < selX || x >= selX + selWidth || y < selY || y >= selY + selLength) {
        selecting = true;
        selX = x;
        selY = y;
        selWidth = 1;
        selLength = 1;
        updateSelection(event);
    }
*/
}

/*
  Update the selected range of map tiles.
  Called when the mouse is over the MapScene with the left button held down.
*/
void MapScene::updateSelection(QGraphicsSceneMouseEvent *event) {
    /*
    int x = selX;
    int y = selY;

    // if event is not null, this was triggered by a mouse action
    // and so the selection area should be updated
    if (event) {
        QPointF pos = event->scenePos();

        x = pos.x() / TILE_SIZE;
        y = pos.y() / TILE_SIZE;

        // ignore invalid mouseover/click positions
        // (use the floating point X coord to avoid roundoff stupidness)
        if (x >= level->width || y >= level->height || pos.x() < 0 || y < 0)
            return;

        // update the selection size
        if (x >= selX)
            selWidth = x - selX + 1;
        else
            selWidth = x - selX - 1;
        if (y >= selY)
            selLength = y - selY + 1;
        else
            selLength = y - selY - 1;
    }

    if (selWidth == 0 || selLength == 0) return;

    int top = std::min(y, selY);
    int left = std::min(x, selX);

    if (event)
        emit statusMessage(QString("Selected (%1, %2) to (%3, %4)")
                           .arg(left).arg(top)
                           .arg(left + abs(selWidth) - 1)
                           .arg(top + abs(selLength) - 1));

    // also, pass the mouseover coords to the main window
    emit mouseOverTile(x, y);
    */
}

/*
  Display information about a map tile being hovered over.
  Called when the mouse is over the MapScene without the left button held down.
*/
void MapScene::showTileInfo(QGraphicsSceneMouseEvent *event) {
    /*
    QPointF pos = event->scenePos();
    // if hte mouse is moved onto a different tile, erase the old one
    // and draw the new one
    if ((pos.x() / TILE_SIZE) != tileX || (pos.y() / TILE_SIZE) != tileY) {
        tileX = pos.x() / TILE_SIZE;
        tileY = pos.y() / TILE_SIZE;

        // ignore invalid mouseover positions
        // (use the floating point coords to avoid roundoff stupidness)
        if (x >= level->width || y >= level->height || pos.x() < 0 || y < 0) {

            uint8_t tile = level->tiles[tileY][tileX];

            // show tile contents on the status bar
            QString stat(QString("(%1, %2) tile %3 (%4)").arg(tileX).arg(tileY)
                         .arg(hexFormat(tile, 2))
                         .arg(tileType(tilesets[level->tileset][tile].action)));

            emit statusMessage(stat);
        } else {
            tileX = -1;
            tileY = -1;

            emit statusMessage("");
        }

        // also, pass the mouseover coords to the main window
        emit mouseOverTile(tileX, tileY);
    }
    */
}

/*
  Remove the selection pixmap from the scene.
*/
void MapScene::cancelSelection() {
    selecting = false;
    selWidth = 0;
    selLength = 0;
    selX = 0;
    selY = 0;
}

void MapScene::setShowCollision(bool on) {
    showCollision = on;
    update();
}

void MapScene::setShowFGDecor(bool on) {
    showVisual[0] = on;
    update();
}

void MapScene::setShowTerrain(bool on) {
    showVisual[1] = on;
    update();
}

void MapScene::setShowBGDecor(bool on) {
    showVisual[2] = on;
    update();
}

void MapScene::setShowBreakable(bool on) {
    showBreakable = on;
    update();
}

void MapScene::setShowObjects(bool on) {
    showObjects = on;
    update();
}

void MapScene::setShowItems(bool on) {
    showItems = on;
    update();
}

void MapScene::setShowEnemies(bool on) {
    showEnemies = on;
    update();
}

void MapScene::drawBackground(QPainter *painter, const QRectF &rect) {
    QRectF rec = sceneRect() & rect;

    if (rec.isNull())
        return;

    for (uint y = rec.top() / TILE_SIZE; y < rec.bottom() / TILE_SIZE; y++) {
        for (uint x = rec.left() / TILE_SIZE; x < rec.right() / TILE_SIZE; x++) {
            // TODO: draw anything (depending on which data section is selected)

            // draw data4 parts 1-3 here (visual)
            for (int i = 2; i >= 0; i--) {
                if (showVisual[i] && level->blocks[y][x].visual[i].first >= 0) {
                    // (TODO: colors / tile numbers)
                    QColor color;
                    color.setHsv(20 * (level->blocks[y][x].visual[i].first) & 0xFF,
                                 255,
                                 255,
                                 i == 1 ? 255 : 128);
                    painter->fillRect(x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE,
                                     color);
                }
            }

            // draw data3 (collision)
            if (showCollision && level->blocks[y][x].collision > 0) {
                // (TODO: colors / tile numbers)
                QColor color;
                color.setHsv(20 * (level->blocks[y][x].collision - 1) & 0xFF,
                             //20 * (level->blocks[y][x].data3 >> 8) & 0xFF,
                             255,
                             255);
                painter->fillRect(x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE,
                                 color);
            }

            // draw data1 (breakables)
            if (showBreakable && level->blocks[y][x].breakable > -1) {
                // (TODO: colors / tile numbers)
                QColor color;
                color.setHsv(20 * (level->blocks[y][x].breakable) & 0xFF,
                             255,
                             255);
                painter->fillRect(x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE,
                                 color);
            }
        }
    }

}

void MapScene::drawForeground(QPainter *painter, const QRectF& /* rect */) {
    // highlight tile under cursor
    if (tileX >= level->width && tileY < level->height && tileX > 0 && tileY > 0) {

        painter->fillRect(tileX * TILE_SIZE, tileY * TILE_SIZE, TILE_SIZE, TILE_SIZE,
                         MapScene::infoBackColor);
    }

    // draw selection
    if (selWidth != 0 && selLength != 0) {
        // account for selections in either negative direction
        int selLeft = qMin(selX, selX + selWidth + 1);
        int selTop  = qMin(selY, selY + selLength + 1);
        QRect selArea(selLeft * TILE_SIZE, selTop * TILE_SIZE, abs(selWidth) * TILE_SIZE, abs(selLength) * TILE_SIZE);
        painter->fillRect(selArea, MapScene::selectionColor);
    }

    // draw objects (add a toggle for this later)
    // for now just write their names
    if (showObjects) for (uint i = 0; i < level->objects.size(); i++) {
        const object_t &obj = level->objects[i];

        QString infoText = level->objectNames[obj.type];
        QRect infoRect = MapScene::infoFontMetrics.boundingRect(infoText);
        double objX = (double)obj.x / 16 * TILE_SIZE;
        // invert Y-axis
        double objY = double(16 * level->height - obj.y) / 16 * TILE_SIZE;

        painter->fillRect(objX, objY - infoRect.height() + MAP_TEXT_PAD_V,
                         infoRect.width() + 2 * MAP_TEXT_PAD_H, infoRect.height() + MAP_TEXT_PAD_V,
                         MapScene::objectColor);
        painter->setFont(MapScene::infoFont);
        painter->drawText(objX, objY - infoRect.height() + 2 * MAP_TEXT_PAD_V,
                          infoRect.width() + 2 * MAP_TEXT_PAD_H, infoRect.height() + 2 * MAP_TEXT_PAD_V,
                          0, infoText);
    }

    // draw items
    if (showItems) for (uint i = 0; i < level->items.size(); i++) {
            const item_t &obj = level->items[i];

            QString infoText = QString("Item");
            QRect infoRect = MapScene::infoFontMetrics.boundingRect(infoText);
            double objX = (double)obj.x / 16 * TILE_SIZE;
            // invert Y-axis
            double objY = double(16 * level->height - obj.y) / 16 * TILE_SIZE;

            painter->fillRect(objX, objY - infoRect.height() + MAP_TEXT_PAD_V,
                             infoRect.width() + 2 * MAP_TEXT_PAD_H, infoRect.height() + MAP_TEXT_PAD_V,
                             MapScene::itemColor);
            painter->setFont(MapScene::infoFont);
            painter->drawText(objX, objY - infoRect.height() + 2 * MAP_TEXT_PAD_V,
                              infoRect.width() + 2 * MAP_TEXT_PAD_H, infoRect.height() + 2 * MAP_TEXT_PAD_V,
                              0, infoText);
    }

    // draw enemies
    if (showEnemies) for (uint i = 0; i < level->enemies.size(); i++) {
            const enemy_t &obj = level->enemies[i];
            const enemytype_t &type = level->enemyTypes[obj.type];

            QString infoText = type.name;
            QRect infoRect = MapScene::infoFontMetrics.boundingRect(infoText);
            double objX = (double)obj.x / 16 * TILE_SIZE;
            // invert Y-axis
            double objY = double(16 * level->height - obj.y) / 16 * TILE_SIZE;

            painter->fillRect(objX, objY - infoRect.height() + MAP_TEXT_PAD_V,
                             infoRect.width() + 2 * MAP_TEXT_PAD_H, infoRect.height() + MAP_TEXT_PAD_V,
                             MapScene::enemyColor);
            painter->setFont(MapScene::infoFont);
            painter->drawText(objX, objY - infoRect.height() + 2 * MAP_TEXT_PAD_V,
                              infoRect.width() + 2 * MAP_TEXT_PAD_H, infoRect.height() + 2 * MAP_TEXT_PAD_V,
                              0, infoText);
    }
}
