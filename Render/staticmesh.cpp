#include "staticmesh.h"

#include <QFile>
#include <QTextStream>



///
/// \brief Constructor.
///
StaticMesh::StaticMesh()
{
    initializeGLFunctions();
    glGenBuffers(1, &m_vertex_buffer);
    glGenBuffers(1, &m_face_buffer);
    m_vertex_count = 0;
    m_face_count = 0;

    if (!m_shader.addShaderFromSourceFile(QGLShader::Vertex, ":/compass.vert")) throw "wtf";
    if (!m_shader.addShaderFromSourceFile(QGLShader::Fragment, ":/compass.frag")) throw "wtf";
    if (!m_shader.link()) throw "wtf";
}



///
/// \brief Destructor.
///
StaticMesh::~StaticMesh()
{
    glDeleteBuffers(1, &m_vertex_buffer);
    glDeleteBuffers(1, &m_face_buffer);
}



///
/// \brief Carga la malla a partir de un fichero.
/// \param fileName Ruta al fichero.
///
void StaticMesh::load( const QString& fileName )
{
    std::vector<VertexData> vertices;
    std::vector<TriangleData> faces;

    // Open the file
    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly)) {
        throw "Couldn't open mesh file";
    }
    QTextStream fileText(&file);

    // Read the vertices
    int nv = 0;
    while(true) {
        auto line = fileText.readLine();
        if( line.isNull() ) {
            throw "Incomplete file";
        }
        else if( (line.isEmpty()) || (line[0] == '#') ) {
            continue;
        }
        else {
            nv = line.toInt();
            break;
        }
    }
    for( int i=0 ; i<nv ; ++i ) {
        auto line = fileText.readLine();
        if( line.isNull() ) {
            throw "Incomplete file";
        }
        else if( (line.isEmpty()) || (line[0] == '#') ) {
            continue;
        }
        else {
            auto fields = line.split(" ", QString::SkipEmptyParts);
            float px = fields[0].toFloat();
            float py = fields[1].toFloat();
            float pz = fields[2].toFloat();
            float nx = fields[3].toFloat();
            float ny = fields[4].toFloat();
            float nz = fields[5].toFloat();
            float red = fields[6].toInt();
            float green = fields[7].toInt();
            float blue = fields[8].toInt();
            float alpha = fields[9].toInt();
            vertices.emplace_back(QVector3D(px, py, pz), QVector3D(nx, ny, nz), RGBA(red, green, blue, alpha));
        }
    }

    // Read the triangles
    int nf = 0;
    while(true) {
        auto line = fileText.readLine();
        if( line.isNull() ) {
            throw "Incomplete file";
        }
        else if( (line.isEmpty()) || (line[0] == '#') ) {
            continue;
        }
        else {
            nf = line.toInt();
            break;
        }
    }
    for( int i=0 ; i<nf ; ++i ) {
        auto line = fileText.readLine();
        if( line.isNull() ) {
            throw "Incomplete file";
        }
        else if( (line.isEmpty()) || (line[0] == '#') ) {
            continue;
        }
        else {
            auto fields = line.split(" ", QString::SkipEmptyParts);
            int v1 = fields[0].toInt();
            int v2 = fields[1].toInt();
            int v3 = fields[2].toInt();
            faces.emplace_back(v1, v2, v3);
        }
    }

    // Transfer the geometry to GPU memory
    return update( vertices, faces );
}



///
/// \brief Actualiza la geometría de la malla.
/// \param vertices Array de vértices.
/// \param faces Array de caras.
///
void StaticMesh::update( const std::vector<VertexData>& vertices, const std::vector<TriangleData>& faces )
{
    // Transfer vertex data
    glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(VertexData), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    m_vertex_count = vertices.size();

    // Transfer index data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_face_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size() * sizeof(TriangleData), faces.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    m_face_count = faces.size();
}



///
/// \brief Renderiza la malla.
/// \param camMatrix Matriz de cámara.
/// \param modelMatrix Matriz de modelo.
///
void StaticMesh::render( const QMatrix4x4& camMatrix, const QMatrix4x4& modelMatrix )
{
    m_shader.bind();
    m_shader.setUniformValue("proj_view_matrix", camMatrix);
    m_shader.setUniformValue("model_matrix", modelMatrix);
    m_shader.setUniformValue("normal_matrix", modelMatrix.normalMatrix());
    m_shader.setUniformValue("light_direction", QVector3D(-0.5773502691896257f, +0.5773502691896257f, -0.5773502691896257f));
    m_shader.setUniformValue("light_color", QVector3D(1.0f, 1.0f, 1.0f));

    // Tell OpenGL which VBOs to use
    glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer);

    // Tell OpenGL programmable pipeline how to locate vertex position data
    int vertexLocation = m_shader.attributeLocation("v_position");
    m_shader.enableAttributeArray(vertexLocation);
    glVertexAttribPointer(vertexLocation, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (const void *)offsetof(VertexData, m_position));

    // Tell OpenGL programmable pipeline how to locate vertex normal data
    int normalLocation = m_shader.attributeLocation("v_normal");
    m_shader.enableAttributeArray(normalLocation);
    glVertexAttribPointer(normalLocation, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (const void *)offsetof(VertexData, m_normal));

    // Tell OpenGL programmable pipeline how to locate vertex texture coordinate data
    int colorLocation = m_shader.attributeLocation("v_color");
    m_shader.enableAttributeArray(colorLocation);
    glVertexAttribPointer(colorLocation, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(VertexData), (const void *)offsetof(VertexData, m_color));

    // Draw the indexed triangles
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_face_buffer);
    glDrawElements(GL_TRIANGLES, 3 * m_face_count, GL_UNSIGNED_SHORT, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
