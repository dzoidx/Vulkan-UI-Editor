#pragma once
#include "Scene.h"
#include "Formats/JsonTokenizer.h"

class SceneJson
{
public:
	SceneJson(String& data);
	bool IsValid() const { return !_invalid && scene_.Nodes.GetLen() > 0; }
	uint32 GetErrorLine() const { return _errorLine; }
	Scene& GetScene() { return scene_; }
private:
	void Load();
	void Error();
private:
	JsonTokenizer tokenizer_;
	bool _invalid;
	uint32 _errorLine;
	Scene scene_;
};
