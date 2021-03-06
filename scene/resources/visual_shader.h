/*************************************************************************/
/*  visual_shader.h                                                      */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2018 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2018 Godot Engine contributors (cf. AUTHORS.md)    */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#ifndef VISUAL_SHADER_H
#define VISUAL_SHADER_H

#include "scene/resources/shader.h"
#include "string_builder.h"

class VisualShaderNodeUniform;
class VisualShaderNode;

class VisualShader : public Shader {
	GDCLASS(VisualShader, Shader)
public:
	enum Type {
		TYPE_VERTEX,
		TYPE_FRAGMENT,
		TYPE_LIGHT,
		TYPE_MAX
	};

	struct Connection {
		int from_node;
		int from_port;
		int to_node;
		int to_port;
	};

	struct DefaultTextureParam {
		StringName name;
		Ref<Texture> param;
	};

private:
	struct Node {
		Ref<VisualShaderNode> node;
		Vector2 position;
	};

	struct Graph {
		Map<int, Node> nodes;
		List<Connection> connections;
	} graph[TYPE_MAX];

	Shader::Mode shader_mode;

	Array _get_node_connections(Type p_type) const;

	Vector2 graph_offset;

	struct RenderModeEnums {
		Shader::Mode mode;
		const char *string;
	};

	HashMap<String, int> modes;
	Set<StringName> flags;

	static RenderModeEnums render_mode_enums[];

	volatile mutable bool dirty;
	void _queue_update();

	union ConnectionKey {

		struct {
			uint64_t node : 32;
			uint64_t port : 32;
		};
		uint64_t key;
		bool operator<(const ConnectionKey &p_key) const {
			return key < p_key.key;
		}
	};

	Error _write_node(Type p_type, StringBuilder &global_code, StringBuilder &code, Vector<DefaultTextureParam> &def_tex_params, const VMap<ConnectionKey, const List<Connection>::Element *> &input_connections, const VMap<ConnectionKey, const List<Connection>::Element *> &output_connections, int node, Set<int> &processed, bool for_preview) const;

	void _input_type_changed(Type p_type, int p_id);

protected:
	virtual void _update_shader() const;
	static void _bind_methods();

	bool _set(const StringName &p_name, const Variant &p_value);
	bool _get(const StringName &p_name, Variant &r_ret) const;
	void _get_property_list(List<PropertyInfo> *p_list) const;

public:
	enum {
		NODE_ID_INVALID = -1,
		NODE_ID_OUTPUT = 0,
	};

	void add_node(Type p_type, const Ref<VisualShaderNode> &p_node, const Vector2 &p_position, int p_id);
	void set_node_position(Type p_type, int p_id, const Vector2 &p_position);

	Vector2 get_node_position(Type p_type, int p_id) const;
	Ref<VisualShaderNode> get_node(Type p_type, int p_id) const;

	Vector<int> get_node_list(Type p_type) const;
	int get_valid_node_id(Type p_type) const;

	int find_node_id(Type p_type, const Ref<VisualShaderNode> &p_node) const;
	void remove_node(Type p_type, int p_id);

	bool is_node_connection(Type p_type, int p_from_node, int p_from_port, int p_to_node, int p_to_port) const;
	bool can_connect_nodes(Type p_type, int p_from_node, int p_from_port, int p_to_node, int p_to_port) const;
	Error connect_nodes(Type p_type, int p_from_node, int p_from_port, int p_to_node, int p_to_port);
	void disconnect_nodes(Type p_type, int p_from_node, int p_from_port, int p_to_node, int p_to_port);

	void get_node_connections(Type p_type, List<Connection> *r_connections) const;

	void set_mode(Mode p_mode);
	virtual Mode get_mode() const;

	void set_graph_offset(const Vector2 &p_offset);
	Vector2 get_graph_offset() const;

	String generate_preview_shader(Type p_type, int p_node, int p_port, Vector<DefaultTextureParam> &r_default_tex_params) const;

	String validate_uniform_name(const String &p_name, const Ref<VisualShaderNodeUniform> &p_uniform) const;

	VisualShader();
};

VARIANT_ENUM_CAST(VisualShader::Type)
///
///
///

class VisualShaderNode : public Resource {
	GDCLASS(VisualShaderNode, Resource)

	int port_preview;

	Map<int, Variant> default_input_values;

	Array _get_default_input_values() const;
	void _set_default_input_values(const Array &p_values);

protected:
	static void _bind_methods();

public:
	enum PortType {
		PORT_TYPE_SCALAR,
		PORT_TYPE_VECTOR,
		PORT_TYPE_TRANSFORM,
	};

	virtual String get_caption() const = 0;

	virtual int get_input_port_count() const = 0;
	virtual PortType get_input_port_type(int p_port) const = 0;
	virtual String get_input_port_name(int p_port) const = 0;

	void set_input_port_default_value(int p_port, const Variant &p_value);
	Variant get_input_port_default_value(int p_port) const; // if NIL (default if node does not set anything) is returned, it means no default value is wanted if disconnected, thus no input var must be supplied (empty string will be supplied)

	virtual int get_output_port_count() const = 0;
	virtual PortType get_output_port_type(int p_port) const = 0;
	virtual String get_output_port_name(int p_port) const = 0;

	void set_output_port_for_preview(int p_index);
	int get_output_port_for_preview() const;

	virtual bool is_port_separator(int p_index) const;

	virtual Vector<StringName> get_editable_properties() const;

	virtual Vector<VisualShader::DefaultTextureParam> get_default_texture_parameters(VisualShader::Type p_type, int p_id) const;
	virtual String generate_global(Shader::Mode p_mode, VisualShader::Type p_type, int p_id) const;
	virtual String generate_code(Shader::Mode p_mode, VisualShader::Type p_type, int p_id, const String *p_input_vars, const String *p_output_vars) const = 0; //if no output is connected, the output var passed will be empty. if no input is connected and input is NIL, the input var passed will be empty

	virtual String get_warning(Shader::Mode p_mode, VisualShader::Type p_type) const;

	VisualShaderNode();
};
/////

class VisualShaderNodeInput : public VisualShaderNode {
	GDCLASS(VisualShaderNodeInput, VisualShaderNode)

	friend class VisualShader;
	VisualShader::Type shader_type;
	Shader::Mode shader_mode;

	struct Port {
		Shader::Mode mode;
		VisualShader::Type shader_type;
		PortType type;
		const char *name;
		const char *string;
	};

	static const Port ports[];
	static const Port preview_ports[];

	String input_name;

protected:
	static void _bind_methods();
	void _validate_property(PropertyInfo &property) const;

public:
	virtual int get_input_port_count() const;
	virtual PortType get_input_port_type(int p_port) const;
	virtual String get_input_port_name(int p_port) const;

	virtual int get_output_port_count() const;
	virtual PortType get_output_port_type(int p_port) const;
	virtual String get_output_port_name(int p_port) const;

	virtual String get_caption() const;

	virtual String generate_code_for_preview(VisualShader::Type p_type, int p_id, const String *p_input_vars, const String *p_output_vars) const;
	virtual String generate_code(Shader::Mode p_mode, VisualShader::Type p_type, int p_id, const String *p_input_vars, const String *p_output_vars) const;

	void set_input_name(String p_name);
	String get_input_name() const;

	int get_input_index_count() const;
	PortType get_input_index_type(int p_index) const;
	String get_input_index_name(int p_index) const;

	PortType get_input_type_by_name(String p_name) const;

	virtual Vector<StringName> get_editable_properties() const;

	VisualShaderNodeInput();
};

///

class VisualShaderNodeOutput : public VisualShaderNode {
	GDCLASS(VisualShaderNodeOutput, VisualShaderNode)
public:
	friend class VisualShader;
	VisualShader::Type shader_type;
	Shader::Mode shader_mode;

	struct Port {
		Shader::Mode mode;
		VisualShader::Type shader_type;
		PortType type;
		const char *name;
		const char *string;
	};

	static const Port ports[];

public:
	virtual int get_input_port_count() const;
	virtual PortType get_input_port_type(int p_port) const;
	virtual String get_input_port_name(int p_port) const;
	Variant get_input_port_default_value(int p_port) const;

	virtual int get_output_port_count() const;
	virtual PortType get_output_port_type(int p_port) const;
	virtual String get_output_port_name(int p_port) const;

	virtual bool is_port_separator(int p_index) const;

	virtual String get_caption() const;

	virtual String generate_code(Shader::Mode p_mode, VisualShader::Type p_type, int p_id, const String *p_input_vars, const String *p_output_vars) const;

	VisualShaderNodeOutput();
};

class VisualShaderNodeUniform : public VisualShaderNode {
	GDCLASS(VisualShaderNodeUniform, VisualShaderNode)

	String uniform_name;

protected:
	static void _bind_methods();

public:
	void set_uniform_name(const String &p_name);
	String get_uniform_name() const;

	VisualShaderNodeUniform();
};

#endif // VISUAL_SHADER_H
