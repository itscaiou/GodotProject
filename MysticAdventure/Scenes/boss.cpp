extends CharacterBody2D

signal died

const DISPARO := preload("res://Poderes/disparos_magicos.tscn")
const RAJADA := preload("res://Poderes/rajada_gelada.tscn")
const FOGO := preload("res://Poderes/bola_de_fogo.tscn")
const FENOMENAL := preload("res://Poderes/poder_fenomenal.tscn")

@export var speed: float = 50.0

@export_group("Health")
@export var max_health: int = 350
@export var life: int = 350

@export var damage: int = 2
@export var follow_range: float = 700.0
@export var attack_range: float = 55.0
@export var cast_range: float = 280.0

@export var attack_windup: float = 0.8
@export var min_action_cooldown: float = 2.0
@export var max_action_cooldown: float = 3.5

@export var burn_damage: int = 1

var player: Node2D = null
var is_stunned := false
var is_attacking := false
var is_dead := false
var is_frozen := false
var is_burning := false
var is_marked := false
var can_act := true
var facing_direction := -1
var queued_action := ""

func _ready():
	add_to_group("enemies")
	randomize()
	player = get_tree().get_first_node_in_group("player")
	life = max_health

func _physics_process(delta):
	if is_dead:
		return
	
	if player == null:
		velocity.x = 0
		play_if_not("idle")
		move_and_slide()
		return
	
	if is_frozen:
		velocity.x = 0
		play_if_not("idle")
		move_and_slide()
		return
	
	if is_stunned:
		velocity.x = 0
		play_if_not("idle")
		move_and_slide()
		return
	
	if is_attacking:
		velocity.x = 0
		move_and_slide()
		return
	
	var distance_x = player.global_position.x - global_position.x
	var distance_abs = abs(distance_x)
	
	if distance_x > 0:
		facing_direction = 1
	else:
		facing_direction = -1
	
	$AnimatedSprite2D.flip_h = facing_direction < 0
	update_spell_point()
	
	if can_act and distance_abs <= cast_range:
		start_action(distance_abs)
	elif distance_abs <= follow_range:
		velocity.x = speed * facing_direction
		play_if_not("run")
	else:
		velocity.x = 0
		play_if_not("idle")
	
	move_and_slide()

func start_action(distance_abs: float):
	if is_attacking or is_dead or is_stunned or is_frozen or not can_act:
		return
	
	is_attacking = true
	can_act = false
	velocity.x = 0
	
	if distance_abs <= attack_range:
		var action_roll = randi() % 100
		if action_roll < 40:
			queued_action = "melee"
		else:
			queued_action = "spell"
	else:
		queued_action = "spell"
	
	play_if_not("attack")
	$AttackTimer.start(attack_windup)

func perform_action():
	if player == null or is_dead:
		return
	
	match queued_action:
		"melee":
			deal_damage_to_player()
		"spell":
			cast_random_spell()
	
	queued_action = ""
	is_attacking = false
	
	var cooldown = randf_range(min_action_cooldown, max_action_cooldown)
	print("Boss entrou em cooldown por ", cooldown, " segundos")
	$CooldownTimer.start(cooldown)

func deal_damage_to_player():
	if player != null and player.has_method("take_damage"):
		if abs(player.global_position.x - global_position.x) <= attack_range + 10:
			player.take_damage(damage)
			print("Boss acertou ataque corpo a corpo")

func cast_random_spell():
	var spell_index = randi() % 4
	
	match spell_index:
		0:
			spawn_disparo()
		1:
			spawn_rajada()
		2:
			spawn_fogo()
		3:
			spawn_fenomenal()

func spawn_disparo():
	var spell_instance = DISPARO.instantiate()
	spell_instance.global_position = $SpellPoint.global_position
	spell_instance.setup(facing_direction, 4, "", 0.0, self)
	get_parent().add_child(spell_instance)
	print("Boss lançou disparo mágico")

func spawn_rajada():
	var spell_instance = RAJADA.instantiate()
	spell_instance.global_position = $SpellPoint.global_position
	spell_instance.setup(facing_direction, 15, "freeze", 5.0, self)
	get_parent().add_child(spell_instance)
	print("Boss lançou rajada gelada")

func spawn_fogo():
	var spell_instance = FOGO.instantiate()
	spell_instance.global_position = $SpellPoint.global_position
	spell_instance.setup(facing_direction, 20, "burn", 4.0, self)
	get_parent().add_child(spell_instance)
	print("Boss lançou bola de fogo")

func spawn_fenomenal():
	var spell_instance = FENOMENAL.instantiate()
	spell_instance.global_position = $SpellPoint.global_position
	spell_instance.setup(facing_direction, 30, "mark", 6.0, self)
	get_parent().add_child(spell_instance)
	print("Boss lançou poder fenomenal")

func update_spell_point():
	$SpellPoint.position.x = abs($SpellPoint.position.x) * facing_direction

func take_hit(amount: int, effect_type: String = "", effect_duration: float = 0.0):
	if is_dead:
		return
	
	var final_damage = amount
	
	if is_marked and effect_type != "mark":
		final_damage += 2
		is_marked = false
		$MarkTimer.stop()
		print("Marca consumida! Dano aumentado.")
	
	take_damage(final_damage)
	
	match effect_type:
		"freeze":
			apply_freeze(effect_duration)
		"burn":
			apply_burn(effect_duration)
		"mark":
			apply_mark(effect_duration)

func take_damage(amount: int):
	if is_dead:
		return
	
	life -= amount
	if life < 0:
		life = 0
	
	print("Boss tomou dano:", amount)
	print("Vida da boss:", life)
	
	if life <= 0:
		die()
		return
	
	is_stunned = true
	is_attacking = false
	velocity = Vector2.ZERO
	
	$AttackTimer.stop()
	$CooldownTimer.stop()
	$StunTimer.start()
	play_if_not("idle")

func apply_freeze(duration: float):
	if is_dead:
		return
	
	is_frozen = true
	is_attacking = false
	velocity = Vector2.ZERO
	$AttackTimer.stop()
	$CooldownTimer.stop()
	$FreezeTimer.start(duration)
	print("Boss congelada por ", duration, " segundos")

func apply_burn(duration: float):
	if is_dead:
		return
	
	is_burning = true
	$BurnTimer.start(duration)
	$BurnTickTimer.start(1.0)
	print("Boss queimando por ", duration, " segundos")

func apply_mark(duration: float):
	if is_dead:
		return
	
	is_marked = true
	$MarkTimer.start(duration)
	print("Boss marcada por ", duration, " segundos")

func die():
	is_dead = true
	is_attacking = false
	is_stunned = false
	is_frozen = false
	is_burning = false
	is_marked = false
	can_act = false
	velocity = Vector2.ZERO
	$CollisionShape2D.disabled = true
	emit_signal("died")
	play_if_not("dead")

func play_if_not(anim_name: String):
	if $AnimatedSprite2D.animation != anim_name:
		$AnimatedSprite2D.play(anim_name)

func _on_stun_timer_timeout() -> void:
	if is_dead:
		return
	is_stunned = false

func _on_attack_timer_timeout() -> void:
	if is_dead:
		return
	perform_action()

func _on_cooldown_timer_timeout() -> void:
	if is_dead:
		return
	can_act = true
	print("Boss pode agir novamente")

func _on_freeze_timer_timeout() -> void:
	if is_dead:
		return
	
	is_frozen = false
	can_act = true
	print("Boss descongelou")

func _on_burn_timer_timeout() -> void:
	if is_dead:
		return
	
	is_burning = false
	$BurnTickTimer.stop()
	print("Queimadura da boss terminou")

func _on_burn_tick_timer_timeout() -> void:
	if is_dead:
		return
	
	if is_burning:
		take_damage(burn_damage)
		print("Queimadura causou ", burn_damage, " de dano na boss")

func _on_mark_timer_timeout() -> void:
	if is_dead:
		return
	
	is_marked = false
	print("Marca da boss expirou")

func _on_animated_sprite_2d_animation_finished() -> void:
	if $AnimatedSprite2D.animation == "dead":
		queue_free()
