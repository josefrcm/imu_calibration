#include "types.h"

#include "eigen3/Eigen/Dense"



///
/// \brief Ajusta una elipsoide a una nube de puntos.
/// \param data Nube de puntos, potencialmente con ruido.
/// \return Matriz de corrección que, al multiplicar cada punto, da una esfera de radio 1.
///
QMatrix4x4 FitAlignedEllipsoid(const std::vector<QVector3D>& data)
{
    // Si no hay suficientes datos, se devuelve una matriz identidad
    if (data.size() < 6)
    {
        return QMatrix4x4();
    }

    // Ajusta una elipsoide del tipo Ax^2 + By^2 + Cz^2 + 2Gx + 2Hy + 2Iz = 1
    Eigen::MatrixXd equation_params( data.size(), 6 );
    for (size_t i = 0; i < data.size(); ++i)
    {
        auto row = data[i];
        equation_params.row(i) <<
            row.x()*row.x(),
            row.y()*row.y(),
            row.z()*row.z(),
            2*row.x(),
            2*row.y(),
            2*row.z();
    }

    // Resuelve el sistema de ecuaciones
    Eigen::MatrixXd mA = equation_params.transpose() * equation_params;
    Eigen::VectorXd mB = equation_params.colwise().sum();
    Eigen::VectorXd v1 = mA.fullPivLu().solve(mB);
    Eigen::VectorXd v2(9);
    v2 << v1[0], v1[1], v1[2], 0.0, 0.0, 0.0, v1[3], v1[4], v1[5];

    // Calcula el centro de la elipsoide
    Eigen::Vector3d center(-v2[6] / v2[0], -v2[7] / v2[1], -v2[8] / v2[2]);
    double gam = 1.0 + (v2[6] * v2[6] / v2[0] + v2[7] * v2[7] / v2[1] + v2[8] * v2[8] / v2[2]);
    Eigen::Vector3d radii(sqrt( gam / v2[0] ), sqrt( gam / v2[1] ), sqrt( gam / v2[2] ));
    //Eigen::Vector3f scale(radii.maxCoeff() / radii[0], radii.maxCoeff() / radii[1], radii.maxCoeff() / radii[2]);
    Eigen::Vector3d scale(1.0 / radii[0], 1.0 / radii[1], 1.0 / radii[2]);

    // Matriz de corrección
    return QMatrix4x4(
        scale.x(), 0.0, 0.0, -center.x()*scale.x(),
        0.0, scale.y(), 0.0, -center.y()*scale.y(),
        0.0, 0.0, scale.z(), -center.z()*scale.z(),
        0.0, 0.0, 0.0, 1.0);
}



/*
///
/// \brief Ajusta una elipsoide a una nube de puntos.
/// \param data Nube de puntos, potencialmente con ruido.
/// \return Matriz de corrección que, al multiplicar cada punto, da una esfera de radio 1.
///
QMatrix4x4 FitOrientedEllipsoid(const std::vector<QVector3D>& data)
{
    // Si no hay suficientes datos, se devuelve una matriz identidad
    if (data.size() < 6)
    {
        return QMatrix4x4();
    }

    // Ajusta una elipsoide del tipo Ax^2 + By^2 + Cz^2 + 2Dxy + 2Exz + 2Fyz + 2Gx + 2Hy + 2Iz = 1
    Eigen::MatrixXf equation_params( data.size(), 9 );
    for (size_t i = 0; i < data.size(); ++i)
    {
        auto row = data[i];
        equation_params.row(i) <<
            row.x()*row.x(),
            row.y()*row.y(),
            row.z()*row.z(),
            2*row.x()*row.y(),
            2*row.x()*row.z(),
            2*row.y()*row.z(),
            2*row.x(),
            2*row.y(),
            2*row.z();
    }

    // Resuelve el sistema de ecuaciones
    Eigen::MatrixXf mA = equation_params.transpose() * equation_params;
    Eigen::VectorXf mB = mA.colwise().sum();
    Eigen::VectorXf v = mA.fullPivLu().solve(mB);

    // Forma algebraica de la elipsoide
    Eigen::Matrix4f A;
    A << v[0], v[3], v[4], v[6],
         v[3], v[1], v[5], v[7],
         v[4], v[5], v[2], v[8],
         v[6], v[7], v[8], -1;

    // Calcula el centro de la elipsoide
    Eigen::Vector3f center = -A.block(0,0,3,3).inverse() * Eigen::Vector3f(v[6], v[7], v[8]);

    // Calcula la matriz de traslación
    Eigen::Matrix4f T;
    T << 1, 0, 0, 0,
         0, 1, 0, 0,
         0, 0, 1, 0,
         center[0], center[1], center[2], 1;

    // Traslada al centro
    Eigen::Matrix4f R = T * A * T.transpose();

    // Resuelve el eigenproblema
    auto es = Eigen::EigenSolver<Eigen::MatrixXf>(R.block(0, 0, 3, 3) / -R(3, 3));
    auto evals = es.eigenvalues();
    auto evecs = es.eigenvectors();
    Eigen::Vector3f radii(
        1.0 / sqrt(std::abs(evals(0))),
        1.0 / sqrt(std::abs(evals(1))),
        1.0 / sqrt(std::abs(evals(2))));
    Eigen::Vector3f scale(
        radii.maxCoeff() / radii[0],
        radii.maxCoeff() / radii[1],
        radii.maxCoeff() / radii[2]);

    // Matriz de traslación
    Eigen::Matrix4f mt;
    mt << 1.0, 0.0, 0.0, -center[0],
          0.0, 1.0, 0.0, -center[1],
          0.0, 0.0, 1.0, -center[2],
          0.0, 0.0, 0.0, 1.0;

    // Matriz de rotación
    auto rot = evecs.inverse();
    Eigen::Matrix4f mr;
    mr << std::abs(rot(0,0)), std::abs(rot(0,1)), std::abs(rot(0,2)), 0.0f,
          std::abs(rot(1,0)), std::abs(rot(1,1)), std::abs(rot(1,2)), 0.0f,
          std::abs(rot(2,0)), std::abs(rot(2,1)), std::abs(rot(2,2)), 0.0f,
          0.0f, 0.0f, 0.0f, 1.0f;

    // Matriz de escalado
    Eigen::Matrix4f ms;
    ms << scale[0], 0.0, 0.0, 0.0,
          0.0, scale[1], 0.0, 0.0,
          0.0, 0.0, scale[2], 0.0,
          0.0, 0.0, 0.0, 1.0;

    // Matriz final
    auto what = ms * mr * mt;
    return QMatrix4x4();
}
*/



///
/// \brief Trocea una línea en clave y valores.
/// \param line Línea de texto.
/// \return Par clave-valores.
///
std::tuple<QString, QList<float>> ParseLine(const QString& line)
{
    // Ignora los comentarios y las líneas vacías
    if (line.isNull() || line.isEmpty() || (line[0] == '#'))
        return std::make_tuple(QString(), QList<float>());

    // Trocea la línea en clave y valores
    const auto fields = line.trimmed().split(' ');
    const auto header = fields[0].toLower();
    QList<float> values;
    for( int i=1 ; i<fields.size() ; ++i ) {
        bool ok;
        float val = QString(fields[i]).toFloat(&ok);
        if( ok ) values.append(val);
        else values.append(NAN);
    }
    return std::make_tuple(header, values);
}
