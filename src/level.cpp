#include <QFile>
#include <QMessageBox>
#include "level.h"

#define CHUNK_TABLE 0x14

uint chunkOffset(QFile &file, uint chunk) {
    uint pos;
    file.seek(CHUNK_TABLE + (4 * chunk));
    file.read((char*)&pos, 4);
    return pos;
}

void seekChunk(QFile& file, uint chunk) {
    file.seek(chunkOffset(file, chunk));
}

void LevelData::loadBreakable(QFile &file, uint chunk) {
    uint width, height;

    seekChunk(file, chunk);
    // read width/height
    file.read((char*)&width, 4);
    file.read((char*)&height, 4);

    this->width = width;
    this->height = height;

    this->blocks.resize(height);
    // read info
    for (int y = height - 1; y >= 0; y--) {
        this->blocks[y].resize(width);
        for (uint x = 0; x < width; x++) {
            file.read((char*)&this->blocks[y][x].breakable, 2);
        }
    }
}

void LevelData::loadCollision(QFile &file, uint chunk) {
    uint ptr, width, height;

    seekChunk(file, chunk);
    // read pointer
    file.read((char*)&ptr, 4);
    file.seek(ptr);
    // read info
    file.read((char*)&width, 4);
    file.read((char*)&height, 4);

    // is there a mismatch between the width/height given here and elsewhere?
    if (width != this->width || height != this->height)
        printf("data3 size mismatch: (%u, %u) != (%u, %u)\n",
               this->width, this->height, width, height);

    for (int y = height - 1; y >= 0; y--) {
         for (uint x = 0; x < width; x++) {
             file.read((char*)&this->blocks[y][x].collision, 4);
         }
    }
}

void LevelData::loadVisual(QFile &file, uint chunk) {
    uint width, height;
    uint ptrs[3];

    seekChunk(file, chunk);
    // get two unknown values
    file.read((char*)&this->unknown1, 4);
    file.read((char*)&this->unknown2, 4);
    printf("chunk 4 unknown 1 = 0x%X unknown 2 = 0x%X\n", this->unknown1, this->unknown2);
    // get pointers to body
    file.read((char*)&ptrs[0], 4);
    file.read((char*)&ptrs[1], 4);
    file.read((char*)&ptrs[2], 4);
    // get data
    for (uint i = 0; i < 3; i++) {
        file.seek(ptrs[i]);
        file.read((char*)&width, 4);
        file.read((char*)&height, 4);

        // is there a mismatch between the width/height given here and elsewhere?
        if (width != this->width || height != this->height)
            printf("data4 size mismatch (body %u): (%u, %u) != (%u, %u)\n",
                   i, this->width, this->height, width, height);

        this->blocks.resize(height);
        for (int y = height - 1; y >= 0; y--) {
             for (uint x = 0; x < width && this->width; x++) {
                 this->blocks[y].resize(width);
                 file.read((char*)&this->blocks[y][x].visual[i].first, 2);
                 file.read((char*)&this->blocks[y][x].visual[i].second, 2);
             }
        }
    }

}

void LevelData::loadEnemies(QFile &file, uint chunk) {
    uint count, size, namePtr, tempPtr;

    seekChunk(file, chunk);
    file.read((char*)&count, 4);
    for (uint i = 0; i < count; i++) {
        enemy_t enemy;

        // save position, read name
        file.read((char*)&namePtr, 4);
        tempPtr = file.pos();
        file.seek(namePtr);
        file.read((char*)&size, 4);
        enemy.name = file.read(size);

        // seek back
        file.seek(tempPtr);

        // TODO: update with known fields
        file.read((char*)&enemy.data1[0], 4);
        file.read((char*)&enemy.data1[1], 4);
        file.read((char*)&enemy.data1[2], 4);

        file.read((char*)&enemy.type, 4);
        file.read((char*)&enemy.x, 4);
        file.read((char*)&enemy.y, 4);

        file.read((char*)&enemy.data2[0], 4);
        file.read((char*)&enemy.data2[1], 4);

        this->enemies.push_back(enemy);
    }
}

void LevelData::loadEnemyTypes(QFile &file, uint chunk) {
    uint count, size, namePtr, statePtr, tempPtr;

    seekChunk(file, chunk);
    file.read((char*)&count, 4);
    for (uint i = 0; i < count; i++) {
        enemytype_t type;
        // name pointer
        file.read((char*)&namePtr, 4);
        // state pointer
        file.read((char*)&statePtr, 4);

        // save pos
        tempPtr = file.pos();

        // get name
        file.seek(namePtr);
        file.read((char*)&size, 4);
        type.name = file.read(size);
        // get state
        file.seek(statePtr);
        file.read((char*)&size, 4);
        type.state = file.read(size);

        this->enemyTypes.push_back(type);

        // seek back
        file.seek(tempPtr);
    }
}

void LevelData::loadMusic(QFile &file, uint chunk) {
    uint count, ptr;

    // TODO: other stuff besides filename, maybe
    seekChunk(file, chunk);
    file.read((char*)&ptr, 4);
    file.seek(ptr);
    file.read((char*)&count, 4);
    this->musicName = file.read(count);
}

void LevelData::loadObjects(QFile &file, uint chunk) {
    uint count, size, objListPtr, nameListPtr, tempPtr, namePtr;

    seekChunk(file, chunk);
    file.read((char*)&objListPtr, 4);
    file.read((char*)&nameListPtr, 4);
    // object table
    file.seek(objListPtr);
    file.read((char*)&count, 4);

    this->objects.resize(count);
    for (uint i = 0; i < count; i++) {
        // TODO: update with known fields
        file.read((char*)&this->objects[i].x, 4);
        file.read((char*)&this->objects[i].y, 4);
        file.read((char*)&this->objects[i].type, 4);

        file.read((char*)&this->objects[i].unknown, 4);
        file.read((char*)&this->objects[i].enabled, 4);

        for (uint j = 0; j < 8; j++) {
            file.read((char*)&this->objects[i].params[j], 4);
        }
    }
    // object names
    file.seek(nameListPtr);
    file.read((char*)&count, 4);

    this->objectNames.resize(count);
    for (uint i = 0; i < count; i++) {
        file.read((char*)&namePtr, 4);
        tempPtr = file.pos();
        file.seek(namePtr);
        file.read((char*)&size, 4);
        this->objectNames[i] = file.read(size);
        file.seek(tempPtr);
    }
}

void LevelData::loadItems(QFile &file, uint chunk) {
    uint count;

    seekChunk(file, chunk);
    file.read((char*)&count, 4);
    this->items.resize(count);
    for (uint i = 0; i < count; i++) {
        for (uint j = 0; j < 3; j++) {
            file.read((char*)&this->items[i].data[j], 4);
        }
        file.read((char*)&this->items[i].x, 4);
        file.read((char*)&this->items[i].y, 4);
        file.read((char*)&this->items[i].data2, 4);
    }
}

bool LevelData::open(QFile& file) {
    this->clear();

    if (chunkOffset(file, 9) == 0x12345678) {
        // main game map
        loadBreakable(file, 0);
        // TODO: check if map data 2 is ever actually used
        loadCollision(file, 2);
        loadVisual(file, 3);
        loadEnemies(file, 4);
        loadEnemyTypes(file, 5);
        loadMusic(file, 6);
        loadObjects(file, 7);
        loadItems(file, 8);
    } else if (chunkOffset(file, 5) == 0x12345678) {
        // Kirby Fighters map
        loadBreakable(file, 0);
        loadCollision(file, 1);
        loadVisual(file, 2);
        loadMusic(file, 3);
        loadObjects(file, 4);
    } else {
        QMessageBox::critical(0, "Load Map", "Unrecognized map format.",
                              QMessageBox::Ok);
        return false;
    }

    fflush(stdout);
    return true;
}

void LevelData::clear() {
    this->width = 0;
    this->height = 0;

    this->musicName = "";

    this->blocks.clear();
    this->enemyTypes.clear();
    this->enemies.clear();
    this->objects.clear();
    this->objectNames.clear();
    this->items.clear();
}
