package com.lavioleta.desarrollo.violetaserver.catalogos.objetossistema.repository;

import com.lavioleta.desarrollo.violetaserver.catalogos.objetossistema.entity.GrupoObjeto;
import com.lavioleta.desarrollo.violetaserver.catalogos.objetossistema.entity.ObjetoSistema;
import com.lavioleta.desarrollo.violetaserver.catalogos.objetossistema.entity.Privilegio;
import java.util.List;
import java.util.Optional;
import org.springframework.jdbc.core.simple.JdbcClient;
import org.springframework.stereotype.Repository;

@Repository
public class ObjetosSistemaRepository {

    private final JdbcClient jdbcClient;

    public ObjetosSistemaRepository(JdbcClient jdbcClient) {
        this.jdbcClient = jdbcClient;
    }

    public List<ObjetoSistema> findAll() {
        return jdbcClient.sql("SELECT objeto, nombre, grupo FROM objetossistema ORDER BY objeto, grupo, nombre")
                .query(ObjetoSistema.class)
                .list();
    }

    public Optional<ObjetoSistema> findById(String objeto) {
        return jdbcClient.sql("SELECT objeto, nombre, grupo FROM objetossistema WHERE objeto = :objeto")
                .param("objeto", objeto)
                .query(ObjetoSistema.class)
                .optional();
    }

    public List<Privilegio> findPrivilegiosByObjeto(String objeto) {
        return jdbcClient.sql("SELECT privilegio, objeto, descripcion FROM privilegios WHERE objeto = :objeto")
                .param("objeto", objeto)
                .query(Privilegio.class)
                .list();
    }

    public List<GrupoObjeto> findAllGrupos() {
        return jdbcClient.sql("SELECT grupo, nombre FROM gruposobjetos ORDER BY nombre")
                .query(GrupoObjeto.class)
                .list();
    }

    public void save(ObjetoSistema objetoSistema) {
        jdbcClient.sql("INSERT INTO objetossistema (objeto, nombre, grupo) VALUES (:objeto, :nombre, :grupo)")
                .param("objeto", objetoSistema.getObjeto())
                .param("nombre", objetoSistema.getNombre())
                .param("grupo", objetoSistema.getGrupo())
                .update();
    }

    public void update(ObjetoSistema objetoSistema) {
        jdbcClient.sql("UPDATE objetossistema SET nombre = :nombre, grupo = :grupo WHERE objeto = :objeto")
                .param("nombre", objetoSistema.getNombre())
                .param("grupo", objetoSistema.getGrupo())
                .param("objeto", objetoSistema.getObjeto())
                .update();
    }

    public void savePrivilegio(Privilegio privilegio) {
        jdbcClient.sql("INSERT INTO privilegios (objeto, privilegio, descripcion) VALUES (:objeto, :privilegio, :descripcion)")
                .param("objeto", privilegio.getObjeto())
                .param("privilegio", privilegio.getPrivilegio())
                .param("descripcion", privilegio.getDescripcion())
                .update();
    }

    public void deleteAsignacionPrivilegios(String objeto) {
        jdbcClient.sql("DELETE FROM asignacionprivilegios WHERE objeto = :objeto")
                .param("objeto", objeto)
                .update();
    }

    public void deletePrivilegios(String objeto) {
        jdbcClient.sql("DELETE FROM privilegios WHERE objeto = :objeto")
                .param("objeto", objeto)
                .update();
    }

    public void deleteObjeto(String objeto) {
        jdbcClient.sql("DELETE FROM objetossistema WHERE objeto = :objeto")
                .param("objeto", objeto)
                .update();
    }

    public boolean existsById(String objeto) {
        return jdbcClient.sql("SELECT COUNT(*) FROM objetossistema WHERE objeto = :objeto")
                .param("objeto", objeto)
                .query(Integer.class)
                .single() > 0;
    }

    public boolean existsGrupoById(String grupo) {
        return jdbcClient.sql("SELECT COUNT(*) FROM gruposobjetos WHERE grupo = :grupo")
                .param("grupo", grupo)
                .query(Integer.class)
                .single() > 0;
    }
}
