#include <QFile>
#include "level.h"

#define CHUNK_TABLE 0x14

void seekChunk(QFile& file, uint chunk) {
    uint pos;
    file.seek(CHUNK_TABLE + (4 * chunk));
    file.read((char*)&pos, 4);
    file.seek(pos);
}

void LevelData::open(QFile& file) {
    this->clear();
    uint width, height, count, size, ptr;
    QString str;
    QVector<uint> ptrs;
    ptrs.resize(3);

    // open map data 1 (chunk 1)
    seekChunk(file, 0);
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
            file.read((char*)&this->blocks[y][x].data1, 2);
        }
    }

    // TODO: check if map data 2 is ever actually used

    // open map data 3 (chunk 3)
    seekChunk(file, 2);
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
             file.read((char*)&this->blocks[y][x].data3, 4);
         }
    }

    // open map data 4 (chunk 4)
    seekChunk(file, 3);
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
                 file.read((char*)&this->blocks[y][x].data4[i].first, 2);
                 file.read((char*)&this->blocks[y][x].data4[i].second, 2);
             }
        }
    }

    // open enemy states (chunk 5)
    seekChunk(file, 4);
    file.read((char*)&count, 4);
    for (uint i = 0; i < count; i++) {
        enemystate_t state;
        file.read((char*)&ptrs[0], 4);
        // TODO: update with known fields
        for (uint j = 0; j < 8; j++)
            file.read((char*)&state.data[j], 4);

        // save position, read name
        ptr = file.pos();
        file.seek(ptrs[0]);
        file.read((char*)&size, 4);
        str = file.read(size);

        // add to state map
        this->states[str] = state;

        // seek back
        file.seek(ptr);
    }

    // open enemies (chunk 6)
    seekChunk(file, 5);
    file.read((char*)&count, 4);
    for (uint i = 0; i < count; i++) {
        enemy_t enemy;
        // name pointer
        file.read((char*)&ptrs[0], 4);
        // state pointer
        file.read((char*)&ptrs[1], 4);

        // save pos
        ptr = file.pos();

        // get name
        file.seek(ptrs[0]);
        file.read((char*)&size, 4);
        enemy.name = file.read(size);
        // get state
        file.seek(ptrs[1]);
        file.read((char*)&size, 4);
        enemy.state = file.read(size);

        this->enemies.push_back(enemy);

        // seek back
        file.seek(ptr);
    }

    // open music info (chunk 7)
    // TODO: other stuff besides filename, maybe
    seekChunk(file, 6);
    file.read((char*)&ptr, 4);
    file.seek(ptr);
    file.read((char*)&count, 4);
    this->musicName = file.read(count);

    // open object info (chunk 8)
    seekChunk(file, 7);
    file.read((char*)&ptrs[0], 4);
    file.read((char*)&ptrs[1], 4);
    // object table
    file.seek(ptrs[0]);
    file.read((char*)&count, 4);

    this->objects.resize(count);
    for (uint i = 0; i < count; i++) {
        // TODO: update with known fields
        file.read((char*)&this->objects[i].x, 4);
        file.read((char*)&this->objects[i].y, 4);
        file.read((char*)&this->objects[i].type, 4);
        for (uint j = 0; j < 10; j++) {
            file.read((char*)&this->objects[i].data[j], 4);
        }
    }
    // object names
    file.seek(ptrs[1]);
    file.read((char*)&count, 4);
    ptrs.resize(count);
    this->objectNames.resize(count);
    for (uint i = 0; i < count; i++) {
        file.read((char*)&ptrs[i], 4);
        ptr = file.pos();
        file.seek(ptrs[i]);
        file.read((char*)&size, 4);
        this->objectNames[i] = file.read(size);
        file.seek(ptr);
    }

    // open map data 5 (chunk 9)
    seekChunk(file, 8);
    file.read((char*)&count, 4);
    this->data5.resize(count);
    for (uint i = 0; i < count; i++) {
        for (uint j = 0; j < 3; j++) {
            file.read((char*)&this->data5[i].data[j], 4);
        }
        file.read((char*)&this->data5[i].x, 4);
        file.read((char*)&this->data5[i].y, 4);
        file.read((char*)&this->data5[i].type, 4);
    }

    fflush(stdout);
}

void LevelData::clear() {
    this->width = 0;
    this->height = 0;

    this->musicName = "";

    this->blocks.clear();
    this->states.clear();
    this->enemies.clear();
    this->objects.clear();
    this->objectNames.clear();
    this->data5.clear();
}
