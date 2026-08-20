/*
 * Cliente web do Cofre Inteligente.
 *
 * Versao de desenvolvimento: roda no computador e aponta para o IP do ESP32.
 * A versao embarcada (dentro de esp32_server.ino) usa caminhos relativos,
 * porque ali a propria placa serve a pagina.
 *
 * Para testar: conecte o computador na rede CofreESP32 e abra este index.html.
 */

const SERVIDOR = 'http://192.168.4.1';
const INTERVALO = 1000;   // consulta o estado a cada 1 s

const $ = id => document.getElementById(id);
let falhasSeguidas = 0;

/* ------------------------------------------------------------------ */
/* Consulta o estado do cofre e o registro de eventos                  */
/* ------------------------------------------------------------------ */
async function atualizar() {
  try {
    const status = await (await fetch(SERVIDOR + '/status')).json();

    $('estado').textContent = status.estado.toUpperCase();
    $('estado').className = 'estado ' + status.estado;
    $('meta').textContent =
      'Erros: ' + status.tentativas + '/' + status.maxTentativas +
      '  |  Dígitos digitados: ' + status.digitos +
      '  |  Ativo há ' + status.uptime + 's';

    // Os botoes refletem o que e possivel fazer agora
    $('btnAbrir').disabled  = (status.estado === 'aberto' || status.estado === 'bloqueado');
    $('btnFechar').disabled = (status.estado !== 'aberto');

    const eventos = await (await fetch(SERVIDOR + '/log')).json();
    $('log').innerHTML = eventos.length
      ? eventos.slice().reverse().map(e =>
          `<li><span>${e.tipo}</span><span>${e.origem} &middot; ${e.t}s</span></li>`
        ).join('')
      : '<li class="vazio">Nenhum evento ainda.</li>';

    falhasSeguidas = 0;
    $('conexao').textContent = 'Conectado ao ESP32 em ' + SERVIDOR;

  } catch (erro) {
    // Tratamento de falha de conexao exigido pela Etapa 3 do regulamento
    falhasSeguidas++;
    $('conexao').textContent =
      'Sem resposta do ESP32 (' + falhasSeguidas + ' tentativas). ' +
      'Confirme que o dispositivo está na rede CofreESP32.';
  }
}

/* ------------------------------------------------------------------ */
/* Envia comando de abertura ou fechamento                             */
/* ------------------------------------------------------------------ */
async function comandar(acao) {
  try {
    const resposta = await fetch(SERVIDOR + '/trava?estado=' + acao, { method: 'POST' });
    if (resposta.status === 423) {
      $('conexao').textContent = 'O cofre está bloqueado por tentativas inválidas. Aguarde 30 segundos.';
      return;
    }
    atualizar();
  } catch (erro) {
    $('conexao').textContent = 'Falha ao enviar o comando. Verifique a conexão e tente novamente.';
  }
}

/* ------------------------------------------------------------------ */
/* Troca a senha do teclado, com validacao antes do envio              */
/* ------------------------------------------------------------------ */
async function trocarSenha() {
  const valor = $('novaSenha').value.trim();
  const erro = $('erroSenha');

  if (!valor) {
    erro.textContent = 'Digite a nova senha.';
    return;
  }
  if (!/^\d{4,8}$/.test(valor)) {
    erro.textContent = 'A senha deve ter de 4 a 8 dígitos numéricos.';
    return;
  }

  try {
    const resposta = await fetch(SERVIDOR + '/senha?nova=' + valor, { method: 'POST' });
    const corpo = await resposta.json();
    if (corpo.ok) {
      erro.textContent = '';
      $('novaSenha').value = '';
      $('conexao').textContent = 'Senha atualizada.';
    } else {
      erro.textContent = corpo.erro;
    }
  } catch (e) {
    erro.textContent = 'Não foi possível salvar. Verifique a conexão.';
  }
}

/* ------------------------------------------------------------------ */
/* Ligacoes de eventos e inicio do ciclo de consulta                   */
/* ------------------------------------------------------------------ */
$('btnAbrir').onclick  = () => comandar('abrir');
$('btnFechar').onclick = () => comandar('fechar');
$('btnSenha').onclick  = trocarSenha;
$('novaSenha').oninput = () => { $('erroSenha').textContent = ''; };

atualizar();
setInterval(atualizar, INTERVALO);
