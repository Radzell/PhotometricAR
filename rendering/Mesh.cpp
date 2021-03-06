#include "Mesh.hpp"

namespace rendering
{
	/**
	 * Default constructor. Creates vertex array only.
	 */
	Mesh::Mesh() : RenderableObject()
	{
	}

	Mesh::~Mesh()
	{	}

	/**
	 * Loads a model file and prepares buffers and designed shader program for rendering.
	 * @param path   Model file path
	 * @param shader Object containing an initilized shader
	 */
	void Mesh::load( const char* _path, BaseShader& _shader)
	{
		bool result = this->loadOBJ( _path, this->vertices, this->uvs, this->normals );

		if( not result )
		{
			cout << "ERROR LOADING MODEL FILE\n";
			return ;
		}

		glGenVertexArrays( 1, &this->vaoIdx );
		glBindVertexArray( this->vaoIdx );

		this->programId = _shader.programId;

		glGenBuffers( 1, &this->vertexBuffer );
		glBindBuffer( GL_ARRAY_BUFFER, this->vertexBuffer );
		glBufferData( GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &this->vertices[0], GL_STATIC_DRAW );

		glGenBuffers( 1, &this->normalBuffer );
		glBindBuffer( GL_ARRAY_BUFFER, this->normalBuffer );
		glBufferData( GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &this->normals[0], GL_STATIC_DRAW );

		glBindAttribLocation( this->programId, 0, "in_vertex" );
		glBindAttribLocation( this->programId, 1, "in_normal" );

		glUseProgram( this->programId );
		
		projectionMatLoc = glGetUniformLocation( this->programId, "projection" );
		viewMatLoc = glGetUniformLocation( this->programId, "view" );
		modelMatLoc = glGetUniformLocation( this->programId, "model" );

		this->modelMatrix = scale(mat4(), vec3(0.012f));
	}

	/**
	 * Method to draw mesh according to camera configuration
	 * @param _camera Camera configuration
	 */
	void Mesh::draw( BaseCamera& _camera )
	{
		glUseProgram( this->programId );

		this->vertexAttribute = 0;
		this->normalAttribute = 1;

		glUniformMatrix4fv(
			this->projectionMatLoc,
			1, GL_FALSE,
			value_ptr( _camera.getProjectionMatrix() )
		);

		glUniformMatrix4fv(
			this->viewMatLoc,
			1, GL_FALSE,
			value_ptr( _camera.getViewMatrix() )
		);

		glUniformMatrix4fv(
			this->modelMatLoc,
			1, GL_FALSE,
			value_ptr( this->modelMatrix )
		);

		glEnableVertexAttribArray( this->vertexAttribute );
		glBindBuffer(GL_ARRAY_BUFFER, this->vertexBuffer);
		glVertexAttribPointer(
			this->vertexAttribute,
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		glEnableVertexAttribArray( this->normalAttribute );
		glBindBuffer(GL_ARRAY_BUFFER, this->normalBuffer);
		glVertexAttribPointer(
			this->normalAttribute,            // attribute
			3,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		glDrawArrays(GL_TRIANGLES, 0, this->vertices.size() );

		glDisableVertexAttribArray( this->vertexAttribute );
		glDisableVertexAttribArray( this->normalAttribute );
	}

	/**
	 * Loads a .obj model at the given path. This is an auxiliar function to the public
	 * load() function.
	 * @param  path         Path to the .obj model
	 * @param  out_vertices Vector containing model's vertices, passed by reference
	 * @param  out_uvs      Vector containing model's texture coordinates,
	 *                      passed by reference
	 * @param  out_normals  Vector containing model's normals, passed by reference
	 * @return              True if function was succesfully executed. False otherwise.
	 */
	bool Mesh::loadOBJ(
		const char * path,
		vector<vec3> & out_vertices, 
		vector<vec2> & out_uvs, 
		vector<vec3> & out_normals
	)
	{
		printf("Loading OBJ file %s...\n", path);

		vector<unsigned int> vertexIndices, uvIndices, normalIndices;
		vector<vec3> temp_vertices; 
		vector<vec2> temp_uvs;
		vector<vec3> temp_normals;


		FILE * file = fopen(path, "r");
		if( file == NULL ){
			printf("Impossible to open the file! Are you in the right path?");
			return false;
		}

		while( 1 ){

			char lineHeader[128];
			// read the first word of the line
			int res = fscanf(file, "%s", lineHeader);
			if (res == EOF)
				break; // EOF = End Of File. Quit the loop.

			// else : parse lineHeader
			int err;
			if ( strcmp( lineHeader, "v" ) == 0 ){

				vec3 vertex;
				err = fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z );
				temp_vertices.push_back(vertex);

			}else if ( strcmp( lineHeader, "vt" ) == 0 ){

				vec2 uv;
				err = fscanf(file, "%f %f\n", &uv.x, &uv.y );
				uv.y = -uv.y; // Invert V coordinate since we will only use DDS texture, which are inverted. Remove if you want to use TGA or BMP loaders.
				temp_uvs.push_back(uv);

			}else if ( strcmp( lineHeader, "vn" ) == 0 ){

				vec3 normal;
				err = fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z );
				temp_normals.push_back(normal);

			}else if ( strcmp( lineHeader, "f" ) == 0 ){

				string vertex1, vertex2, vertex3;
				unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];

				int matches = fscanf(
					file, 
					"%d/%d/%d %d/%d/%d %d/%d/%d\n", 
					&vertexIndex[0], 
					&uvIndex[0], 
					&normalIndex[0], 
					&vertexIndex[1], 
					&uvIndex[1], 
					&normalIndex[1], 
					&vertexIndex[2], 
					&uvIndex[2], 
					&normalIndex[2]
				);

				if (matches != 9){
					printf("File can't be read by our simple parser :-( Try exporting with other options\n");
					return false;
				}

				vertexIndices.push_back(vertexIndex[0]);
				vertexIndices.push_back(vertexIndex[1]);
				vertexIndices.push_back(vertexIndex[2]);
				uvIndices    .push_back(uvIndex[0]);
				uvIndices    .push_back(uvIndex[1]);
				uvIndices    .push_back(uvIndex[2]);
				normalIndices.push_back(normalIndex[0]);
				normalIndices.push_back(normalIndex[1]);
				normalIndices.push_back(normalIndex[2]);

			}else{
				// Probably a comment, eat up the rest of the line
				char stupidBuffer[1000];
				char* trash = fgets(stupidBuffer, 1000, file);
			}

		}

		// For each vertex of each triangle
		for( unsigned int i=0; i<vertexIndices.size(); i++ ){

			// Get the indices of its attributes
			unsigned int vertexIndex = vertexIndices[i];
			unsigned int uvIndex = uvIndices[i];
			unsigned int normalIndex = normalIndices[i];
			
			// Get the attributes thanks to the index
			vec3 vertex = temp_vertices[ vertexIndex-1 ];
			vec2 uv = temp_uvs[ uvIndex-1 ];
			vec3 normal = temp_normals[ normalIndex-1 ];
			
			// Put the attributes in buffers
			out_vertices.push_back(vertex);
			out_uvs     .push_back(uv);
			out_normals .push_back(normal);
		
		}

		return true;
	}

}