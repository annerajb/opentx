/*
 * Copyright (C) OpenTX
 *
 * Based on code named
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "modelslist.h"
using std::list;

ModelsList modelslist;

ModelCell::ModelCell(const char * name)
  : buffer(NULL), valid_rfData(false)
{
  strncpy(modelFilename, name, sizeof(modelFilename));
  memset(modelName, 0, sizeof(modelName));
}

ModelCell::~ModelCell()
{
  resetBuffer();
}

void ModelCell::setModelName(char* name)
{
  zchar2str(modelName, name, LEN_MODEL_NAME);
  if (modelName[0] == 0) {
    char * tmp;
    strncpy(modelName, modelFilename, LEN_MODEL_NAME);
    tmp = (char *) memchr(modelName, '.',  LEN_MODEL_NAME);
    if (tmp != NULL)
      *tmp = 0;
  }

  resetBuffer();
}

void ModelCell::resetBuffer()
{
  if (buffer) {
    delete buffer;
    buffer = NULL;
  }
}

const BitmapBuffer * ModelCell::getBuffer()
{
  if (!buffer) {
    loadBitmap();
  }
  return buffer;
}

void ModelCell::loadBitmap()
{
  PACK(struct {
    ModelHeader header;
    TimerData timers[MAX_TIMERS];
  }) partialmodel;
  const char * error = NULL;

  buffer = new BitmapBuffer(BMP_RGB565, MODELCELL_WIDTH, MODELCELL_HEIGHT);
  if (buffer == NULL) {
    return;
  }

  if (strncmp(modelFilename, g_eeGeneral.currModelFilename, LEN_MODEL_FILENAME) == 0) {
    memcpy(&partialmodel.header, &g_model.header, sizeof(partialmodel));
  }
  else {
    error = readModel(modelFilename, (uint8_t *)&partialmodel.header, sizeof(partialmodel));
  }

  buffer->clear(TEXT_BGCOLOR);

  if (error) {
    buffer->drawText(5, 2, "(Invalid Model)", TEXT_COLOR);
    buffer->drawBitmapPattern(5, 23, LBM_LIBRARY_SLOT, TEXT_COLOR);
  }
  else {
    if (modelName[0] == 0)
      setModelName(partialmodel.header.name);

    char timer[LEN_TIMER_STRING];
    buffer->drawSizedText(5, 2, modelName, LEN_MODEL_NAME, SMLSIZE|TEXT_COLOR);
    getTimerString(timer, 0);
    for (uint8_t i = 0; i < MAX_TIMERS; i++) {
      if (partialmodel.timers[i].mode > 0 && partialmodel.timers[i].persistent) {
        getTimerString(timer, partialmodel.timers[i].value);
        break;
      }
    }
    buffer->drawText(101, 40, timer, TEXT_COLOR);
    for (int i=0; i<4; i++) {
      buffer->drawBitmapPattern(104+i*11, 25, LBM_SCORE0, TITLE_BGCOLOR);
    }
    GET_FILENAME(filename, BITMAPS_PATH, partialmodel.header.bitmap, "");
    const BitmapBuffer * bitmap = BitmapBuffer::load(filename);
    if (bitmap) {
      buffer->drawScaledBitmap(bitmap, 5, 24, 56, 32);
      delete bitmap;
    }
    else {
      buffer->drawBitmapPattern(5, 23, LBM_LIBRARY_SLOT, TEXT_COLOR);
    }
  }
  buffer->drawSolidHorizontalLine(5, 19, 143, LINE_COLOR);
}

void ModelCell::save(FIL* file)
{
  f_puts(modelFilename, file);
  f_putc('\n', file);
}

void ModelCell::setRfData(ModelData* model)
{
  for (uint8_t i = 0; i < NUM_MODULES; i++) {
    modelId[i] = model->header.modelId[i];
    setRfModuleData(i, &(model->moduleData[i]));
    TRACE("<%s/%i> : %X,%X,%X",
          strlen(modelName) ? modelName : modelFilename,
          i, moduleData[i].type, moduleData[i].rfProtocol, modelId[i]);
  }
  valid_rfData = true;
}

void ModelCell::setRfModuleData(uint8_t moduleIdx, ModuleData* modData)
{
  moduleData[moduleIdx].type = modData->type;
  if (modData->type != MODULE_TYPE_MULTIMODULE) {
    moduleData[moduleIdx].rfProtocol = (uint8_t)modData->rfProtocol;
  }
  else {
    // do we care here about MM_RF_CUSTOM_SELECTED? probably not...
    moduleData[moduleIdx].rfProtocol = modData->getMultiProtocol(false);
  }
}

bool ModelCell::fetchRfData()
{
  //TODO: use g_model in case fetching data for current model
  //
  char buf[256];
  getModelPath(buf, modelFilename);

  FIL      file;
  uint16_t file_size;

  const char* err = openFile(buf,&file,&file_size);
  if (err) return false;

  FSIZE_t start_offset = f_tell(&file);


  UINT read;
  if ((f_read(&file, buf, LEN_MODEL_NAME, &read) != FR_OK) || (read != LEN_MODEL_NAME))
    goto error;

  setModelName(buf);

  // 1. fetch modelId: NUM_MODULES @ offsetof(ModelHeader, modelId)
  // if (f_lseek(&file, start_offset + offsetof(ModelHeader, modelId)) != FR_OK)
  //   goto error;
  if ((f_read(&file, modelId, NUM_MODULES, &read) != FR_OK) || (read != NUM_MODULES))
    goto error;

  // 2. fetch ModuleData: sizeof(ModuleData)*NUM_MODULES @ offsetof(ModelData, moduleData)
  if (f_lseek(&file, start_offset + offsetof(ModelData, moduleData)) != FR_OK)
    goto error;

  for(uint8_t i=0; i<NUM_MODULES; i++) {
    ModuleData modData;
    if ((f_read(&file, &modData, NUM_MODULES, &read) != FR_OK) || (read != NUM_MODULES))
      goto error;

    setRfModuleData(i, &modData);
  }

  valid_rfData = true;  
  f_close(&file);
  return true;
  
 error:
  f_close(&file);
  return false;  
}

ModelsCategory::ModelsCategory(const char * name)
{
  strncpy(this->name, name, sizeof(this->name));
}

ModelsCategory::~ModelsCategory()
{
  for (list<ModelCell *>::iterator it = begin(); it != end(); ++it) {
    delete *it;
  }
}


ModelCell * ModelsCategory::addModel(const char * name)
{
  ModelCell * result = new ModelCell(name);
  push_back(result);
  return result;
}

void ModelsCategory::removeModel(ModelCell * model)
{
  delete model;
  remove(model);
}

void ModelsCategory::moveModel(ModelCell * model, int8_t step)
{
  ModelsCategory::iterator current = begin();
  for (; current != end(); current++) {
    if (*current == model) {
      break;
    }
  }

  ModelsCategory::iterator new_position = current;
  if (step > 0) {
    while (step >= 0 && new_position != end()) {
      new_position++;
      step--;
    }
  }
  else {
    while (step < 0 && new_position != begin()) {
      new_position--;
      step++;
    }
  }

  insert(new_position, 1, *current);
  erase(current);
}

void ModelsCategory::save(FIL * file)
{
  f_puts("[", file);
  f_puts(name, file);
  f_puts("]", file);
  f_putc('\n', file);
  for (list<ModelCell *>::iterator it = begin(); it != end(); ++it) {
    (*it)->save(file);
  }
}

ModelsList::ModelsList()
{
  init();
}

ModelsList::~ModelsList()
{
  clear();
}

void ModelsList::init()
{
  loaded = false;
  currentCategory = NULL;
  currentModel = NULL;
  modelsCount = 0;
}

void ModelsList::clear()
{
  for (list<ModelsCategory *>::iterator it = categories.begin(); it != categories.end(); ++it) {
    delete *it;
  }
  categories.clear();
}

bool ModelsList::load()
{
  char line[LEN_MODELS_IDX_LINE+1];
  ModelsCategory * category = NULL;

  if (loaded)
    return true;

  FRESULT result = f_open(&file, RADIO_MODELSLIST_PATH, FA_OPEN_EXISTING | FA_READ);
  if (result == FR_OK) {
    while (readNextLine(line, LEN_MODELS_IDX_LINE)) {
      int len = strlen(line); // TODO could be returned by readNextLine
      if (len > 2 && line[0] == '[' && line[len-1] == ']') {
        line[len-1] = '\0';
        category = new ModelsCategory(&line[1]);
        categories.push_back(category);
      }
      else if (len > 0) {

        //char* rf_data_str = cutModelFilename(line);
        ModelCell * model = new ModelCell(line);
        if (!category) {
          category = new ModelsCategory("Models");
          categories.push_back(category);
        }
        category->push_back(model);
        if (!strncmp(line, g_eeGeneral.currModelFilename, LEN_MODEL_FILENAME)) {
          currentCategory = category;
          currentModel = model;
        }
        //parseModulesData(model, rf_data_str);
        //TRACE("model=<%s>, valid_rfData=<%i>",model->modelFilename,model->valid_rfData);
        model->fetchRfData();
        modelsCount += 1;
      }
    }
    f_close(&file);

    if (!getCurrentModel()) {
      TRACE("currentModel is NULL");
    }
  }

  if (categories.size() == 0) {
    category = new ModelsCategory("Models");
    categories.push_back(category);
  }

  loaded = true;
  return true;
}

void ModelsList::save()
{
  FRESULT result = f_open(&file, RADIO_MODELSLIST_PATH, FA_CREATE_ALWAYS | FA_WRITE);
  if (result != FR_OK) {
    return;
  }

  for (list<ModelsCategory *>::iterator it = categories.begin(); it != categories.end(); ++it) {
    (*it)->save(&file);
  }

  f_close(&file);
}

void ModelsList::setCurrentCategorie(ModelsCategory* cat)
{
  currentCategory = cat;
}

void ModelsList::setCurrentModel(ModelCell* cell)
{
  currentModel = cell;
  if (!currentModel->valid_rfData)
    currentModel->fetchRfData();
}

bool ModelsList::readNextLine(char * line, int maxlen)
{
  if (f_gets(line, maxlen, &file) != NULL) {
    int curlen = strlen(line) - 1;
    if (line[curlen] == '\n') { // remove unwanted chars if file was edited using windows
      if (line[curlen - 1] == '\r') {
        line[curlen - 1] = 0;
      }
      else {
        line[curlen] = 0;
      }
    }
    return true;
  }
  return false;
}

ModelsCategory * ModelsList::createCategory()
{
  ModelsCategory * result = new ModelsCategory("Category");
  categories.push_back(result);
  save();
  return result;
}

ModelCell * ModelsList::addModel(ModelsCategory * category, const char * name)
{
  ModelCell * result = category->addModel(name);
  modelsCount++;
  save();
  return result;
}

void ModelsList::removeCategory(ModelsCategory * category)
{
  modelsCount -= category->size();
  delete category;
  categories.remove(category);
}

void ModelsList::removeModel(ModelsCategory * category, ModelCell * model)
{
  category->removeModel(model);
  modelsCount--;
  save();
}

void ModelsList::moveModel(ModelsCategory * category, ModelCell * model, int8_t step)
{
  category->moveModel(model, step);
  save();
}

void ModelsList::moveModel(ModelCell * model, ModelsCategory * previous_category, ModelsCategory * new_category)
{  
  previous_category->remove(model);
  new_category->push_back(model);
  save();
}